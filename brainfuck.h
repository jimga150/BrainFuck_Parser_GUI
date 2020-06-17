#ifndef BRAINFUCK_H
#define BRAINFUCK_H

#include <QObject>
#include <QTextStream>
#include <QElapsedTimer>
#include <QThread>

#define VALIDATE_SETTER(data_name_str) if (this->running){fprintf(stderr, "Program thread running, cannot set "); fprintf(stderr, data_name_str); fprintf(stderr, "\n"); return false;}

struct ui_updates_struct{
    //TODO: probably an input pointer i can display
    bool update_mem_ptr = false;
    uint64_t mem_ptr = 0;
    
    bool update_mem = false;
    QVector<char> memory;
    
    bool update_output = false;
    QString output;
    
    ui_updates_struct(){}
    
    ui_updates_struct(const ui_updates_struct& other){
        this->update_mem_ptr = other.update_mem_ptr;
        this->mem_ptr = other.mem_ptr;
        
        this->update_mem = other.update_mem;
        this->memory = other.memory;
        
        this->update_output = other.update_output;
        this->output = other.output;
    }
    
    ~ui_updates_struct(){
        //do nothing
    }
    
    void reset(){
        
        this->update_mem_ptr = false;
        this->mem_ptr = 0;
        
        this->update_mem = false;
        this->memory.clear();
        
        this->update_output = false;
        this->output = "";
    }
    
    void operator|=(ui_updates_struct& other){
        
        this->update_mem_ptr |= other.update_mem_ptr;
        this->mem_ptr = other.mem_ptr;
        
        this->update_mem |= other.update_mem;
        this->memory = other.memory;
        
        this->update_output |= other.update_output;
        this->output = other.output;
    }
};
Q_DECLARE_METATYPE(ui_updates_struct)

struct program_post_struct{
    
    int error_code = 0;
    QString error_message = "";
    
    QString output = "";
    
    uint64_t memory_access_count = 0;
    uint64_t instruction_count = 0;
    
    uint64_t execution_time = 0; //ms
    uint64_t memory_size = 0;
};

class BrainFuck : public QObject
{
    Q_OBJECT
    
public:
    explicit BrainFuck(QObject *parent = nullptr);
    
    //getters and setters
    QString getProgram(){return this->program;}
    bool setProgram(QString new_program);
    
    bool getMaxMemEnforced(){return this->max_mem_enforced;}
    bool setMaxMemEnforced(bool new_state);
    
    uint64_t getMaxMem(){return this->max_memory;}
    bool setMaxMem(uint64_t new_maxmem);
    
    bool getMaxInstEnforced(){return this->max_instructions_enforced;}
    bool setMaxInstEnforced(bool new_state);
    
    uint64_t getMaxInstructions(){return this->max_instructions;}
    bool setMaxInstructions(uint64_t new_maxinst);
    
    QString getInput(){return this->input;}
    bool setInput(QString new_input);
    
    bool getCurrentState(ui_updates_struct* state);
    
    //utilities
    void reset_program();
    
    
    
    volatile bool stop = false; //To be assigned from other thread to halt program
    volatile bool running = false; //To be monitored from other thread
    
    volatile unsigned long command_delay = 0; //ms    
    
private:
    
    const QString default_brainfuck = 
            "++++++++[>++++[>++>+++>+++>+<<<<-]"
            ">+>->+>>+[<]<-]>>.>>---.+++++++..+"
            "++.>.<<-.>.+++.------.--------.>+.>++.";
    
    QString program = "";
    
    QVector<char> memory;
    uint64_t mem_index = 0;
    
    bool update_ui = false;
    ui_updates_struct ui_updates;
    
    QString input = "";
    QString output = "";
    QString error_message = "";
    
    bool max_instructions_enforced = false;
    uint64_t max_instructions = 0;
    
    bool max_mem_enforced = false;
    uint64_t max_memory = 0;
    
    uint64_t memory_access_count = 0;
    uint64_t instruction_count = 0;
    
    uint64_t execution_time = 0; //ms
    
signals:
    
    void requestUIUpdate(ui_updates_struct updates);
        
    
public slots:
    
    program_post_struct runProgram();
    
};

#endif // BRAINFUCK_H
