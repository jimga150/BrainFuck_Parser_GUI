#include "brainfuck.h"

BrainFuck::BrainFuck(QObject *parent) : QObject(parent){
    
    this->memory.push_back(0);
    this->program = this->default_brainfuck;
}

void BrainFuck::reset_program(){
    this->memory.clear();
    this->memory.push_back(0);
    this->mem_index = 0;
    
    this->output = "";
    
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
        //bracket mismatch
        fprintf(stderr, "Bracket mismatch (last known level: %d, exiting)...\n", looplevel);
        emit this->programExit(1); //TODO: make real error enum
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
    
    //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");	
            
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
    for(
        uint64_t i = 0; 
        
        !this->stop &&
        i < program_length && 
        (!this->max_instructions_enforced || (this->instruction_count != this->max_instructions)); 
        
        i++){
        
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
            
            if (this->mem_index >= this->memory.size()){
                this->memory.push_back(0);
            }
            
            break;
        case '<':

            if (this->mem_index-- == 0){
                //TODO: make useful error message channel
                fprintf(stderr, "BrainFucked!!! (memory index underflow, exiting)\n");
                this->stop = true;
            }
            
            break;
        case '+':
            
            ++this->memory_access_count;
            ++this->memory[this->mem_index];
            
            break;
        case '-':
            
            ++this->memory_access_count;
            --this->memory[this->mem_index];
            
            break;
        case '['://if pointer is 0, jump to command AFTER ending bracket
            
            ++this->memory_access_count;
            
            if (!(this->memory[this->mem_index])){
                i = LUT[i];
            }
            
            break;
        case ']'://jump to complimentary opening bracket if pointer is non-zero
            
            ++this->memory_access_count;
            
            if (this->memory[this->mem_index]){
                i = LUT[i];
            }
            
            break;
        case '.':
            
            ++this->memory_access_count;
            
            this->output.append(this->memory[this->mem_index]);
            
            this->update_output = true;
            this->update_some_ui = true;
            
            break;
        case ',':
            
            ++this->memory_access_count;
            
            if (input.atEnd()){
                fprintf(stderr, "End reached in input stream, exiting...\n");
                this->stop = true;
                break;
            }
            
            input >> this->memory[this->mem_index];
            
            break;
        default:
            //ignore any other input
            break;
        }
        
        if (this->update_some_ui){
            emit this->requestUIUpdate();
            this->update_some_ui = false;
        }
    }
        
    //printf("%s\n", this->output.toUtf8().constData());    
    //printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    
//    fprintf(stdout, "Memory used: %ld\n", this->memory.size());
//    fprintf(stdout, "# instructions: %llu\n", this->instruction_count);
//    fprintf(stdout, "# of memory accesses: %llu\n", this->memory_access_count);
    
//    double mem_access_percent = ((double)this->memory_access_count*100)/((double)this->instruction_count);
//    fprintf(stdout, "%% of instructions that access memory: %0.1f%%\n", mem_access_percent);
    
    int error = this->stop ? 1 : 0;
    
    this->running = false;
    this->stop = false;
    
    emit this->programExit(error);
}