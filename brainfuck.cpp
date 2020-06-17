#include "brainfuck.h"

BrainFuck::BrainFuck(QObject *parent) : QObject(parent){
    
    this->memory.push_back(0);
    this->program = this->default_brainfuck;
}

bool BrainFuck::setProgram(QString new_program){
    if (this->running) return false;
    
    this->program = new_program;
    return true;
}

bool BrainFuck::setMaxMemEnforced(bool new_state){
    if (this->running) return false;
    
    this->max_mem_enforced = new_state;
    return true;
}

bool BrainFuck::setMaxMem(uint64_t new_maxmem){
    if (this->running) return false;
    
    this->max_memory = new_maxmem;
    return true;
}

bool BrainFuck::setMaxInstEnforced(bool new_state){
    if (this->running) return false;
    
    this->max_instructions_enforced = new_state;
    return true;
}

bool BrainFuck::setMaxInstructions(uint64_t new_maxinst){
    if (this->running) return false;
    
    this->max_instructions = new_maxinst;
    return true;
}

bool BrainFuck::setInput(QString new_input){
    if (this->running) return false;
    
    this->input = new_input;
    return true;
}

bool BrainFuck::getCurrentState(ui_updates_struct* state){
    
    if (this->running) return false; //Indicates that current state isnt accessible in this thread, wait for an update
    
    state->mem_ptr = this->mem_index;
    state->memory = this->memory;
    state->output = this->output;
    
    return true;
}

void BrainFuck::reset_program(){
    this->memory.clear();
    this->memory.push_back(0);
    this->mem_index = 0;
    
    this->output = "";
    this->error_message = "";
    
    this->memory_access_count = 0;
    this->instruction_count = 0;
}

void BrainFuck::runProgram(){
    
    this->running = true;
    
    const QString valid_commands = "><+-[].,";
    
    QTextStream input(&(this->input), QIODevice::ReadOnly);
                
//    for (QChar c : this->program){
//        bool valid = false;
//        for (QChar v : valid_commands){
//            if (v == c){
//                valid = true;
//                break;
//            }
//        }
//        if (!valid){
//            this->program.remove(c); //TODO: test this
//        }
//    }
    
    uint program_length = this->program.length();
    char program_chars[program_length];
    for (uint i = 0; i < program_length; ++i){
        program_chars[i] = this->program.toUtf8().constData()[i];
    }
    
//    printf("QString: \t%s\n", this->program.toUtf8().constData());
//    printf("char*: \t%s\n", program_chars);
            
    //build loop list
    long LUT[program_length];
    //printf("%p\n", (void*)LUT);
    int levels[program_length];
    
    int looplevel = 0;
    for(uint i = 0; i < program_length; ++i){
        //printf("Loop level: %d\n", looplevel);
        switch(program_chars[i]){
        case '[':
            levels[i] = ++looplevel;
            break;
        case ']':
            levels[i] = -1*(looplevel--);
            break;
        default:
            levels[i] = 0;
            break;
        }
    }
    
//    printf("Levels: [");
//    for (uint i = 0; i < program_length; ++i){
//        printf("%d, ", levels[i]);
//    }
//    printf("]\n");
        
    if (looplevel){
        program_post_struct result; 
        result.error_code = 1;
        result.error_message = "Bracket mismatch. Last known level: " + QString::number(looplevel);
        emit this->programExit(result); //TODO: this is dirty
    }
    
    //make lookup table for brackets based on loop level list
    //Opening brackets get index of their closing counterpart, and vice-versa
    for(uint i = 0; i < program_length; ++i){
        
        if (levels[i] > 0){
            
            //printf("Found opening bracket: level %ld, character %d\n", LUT[i], i);
            int matching_bracket_id = -1*levels[i];
            
            //printf("finding %d\n", matching_bracket_id);
            for (ulong b=(ulong)i; b < program_length; ++b){
                
                //printf("b loop: b=%lu\n", b);
                if (levels[b] == matching_bracket_id){
                    
                    //printf("Found matching closer at position %lu\n", b);
                    LUT[i] = b;
                    LUT[b] = (long)i;
                    break;
                }
            }
        }
    }
    
    //printf("printing LUT:\n");
    //printLongArray(stdout, LUT, char_count, "%ld ");
                
//#if FALSE //#ifdef RAND_MEM
//    printf("Randomizing memory...\n");
//    //Loop through and randomize all memory
//    for (int i=0; i < memory_size; i++){
//        char rand_num = rand() % (UCHAR_MAX+1);
//        //printf("Setting index %d to %u\n", i, (int)((unsigned char)rand_num));
//        memory[i] = rand_num;
//        //printf("Memory at %d is %x\n", i, ptr[i]);
//    }
//#endif
    
    //printf("Made it to program loop\n");
    
    QElapsedTimer timer;
    timer.start();
    
    for(
        uint64_t i = 0; 
        
        !this->stop &&
        i < program_length && 
        (!this->max_instructions_enforced || (this->instruction_count != this->max_instructions)); 
        
        i++){
        
        if (this->command_delay > 0){
            QThread::msleep(this->command_delay);
        }
        
        ++this->instruction_count;
        
//        printf("%llu: %llu-> %c (%d), mem ptr @ %llu, memory: [", this->instruction_count, i, program_chars[i], (int)program_chars[i], this->mem_index);
//        for (char c : this->memory){
//            printf("%d, ", (int)c);
//        }
//        printf("]\n");
        
        switch (program_chars[i]){
        case '>':
            
            ++this->mem_index;
            
            if (this->max_mem_enforced && (this->mem_index >= this->max_memory)){
                this->stop = true;
                break;
            }
            
            if (this->mem_index >= static_cast<uint64_t>(this->memory.size())){
                this->memory.push_back(0);
            }
            
            this->ui_updates.update_mem_ptr = true;
            this->update_ui = true;
            
            break;
        case '<':

            if (this->mem_index-- == 0){
                this->error_message = "BrainFucked!!! (memory index underflow)";
                this->stop = true;
            }
            
            this->ui_updates.update_mem_ptr = true;
            this->update_ui = true;
            
            break;
        case '+':
            
            ++this->memory[this->mem_index];            
            
            ++this->memory_access_count;
            this->ui_updates.update_mem = true;
            this->update_ui = true;
            
            break;
        case '-':
            
            --this->memory[this->mem_index];            
            
            ++this->memory_access_count;
            this->ui_updates.update_mem = true;
            this->update_ui = true;
            
            break;
        case '['://if pointer is 0, jump to command AFTER ending bracket
            
            if (!(this->memory[this->mem_index])){
                i = LUT[i];
            }
            
            ++this->memory_access_count;
            
            break;
        case ']'://jump to complimentary opening bracket if pointer is non-zero
            
            if (this->memory[this->mem_index]){
                i = LUT[i];
            }
            
            ++this->memory_access_count;
            
            break;
        case '.':
            
            this->output.append(this->memory[this->mem_index]);            
            
            ++this->memory_access_count;            
            this->ui_updates.update_output = true;
            this->update_ui = true;
            
//            printf("Output CMD received\n");
//            fflush(stdout);
            
            break;
        case ',':
            
            if (input.atEnd()){ //TODO: add option to wait for input instead of exiting with error
                this->error_message = "Reached end of input stream";
                this->stop = true;
                break;
            }
            
            input >> this->memory[this->mem_index];
            
            ++this->memory_access_count;
            
            break;
        default:
            //ignore any other input
            break;
        }
        
        if (this->update_ui){
            
            this->ui_updates.mem_ptr = this->mem_index;            
            this->ui_updates.memory = this->memory;            
            this->ui_updates.output = this->output;            
            
            emit this->requestUIUpdate(this->ui_updates);
            this->ui_updates.reset();
            this->update_ui = false;
        }
    }
    
    this->execution_time = timer.elapsed();
    
    int error = this->stop ? 1 : 0;
    if (this->error_message.length() == 0){
        this->error_message = "Program Interrupted";
    }
    
    this->running = false;
    this->stop = false;
    
    program_post_struct result;
    
    result.output = this->output;
    result.error_code = error;
    result.memory_size = this->memory.size();
    result.error_message = this->error_message;
    result.execution_time = this->execution_time;
    result.instruction_count = this->instruction_count;
    result.memory_access_count = this->memory_access_count;
    
    emit this->programExit(result);
}