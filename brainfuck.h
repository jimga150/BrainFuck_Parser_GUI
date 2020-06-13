#ifndef BRAINFUCK_H
#define BRAINFUCK_H

#include <QObject>
#include <QTextStream>
#include <QElapsedTimer>

struct ui_updates_struct{
    //TODO: probably an input pointer i can display
    bool update_mem_ptr = false;
    bool update_mem = false;
    bool update_output = false;
    
    void reset(){
        this->update_mem_ptr = false;
        this->update_mem = false;
        this->update_output = false;
    }
};

class BrainFuck : public QObject
{
    Q_OBJECT
public:
    explicit BrainFuck(QObject *parent = nullptr);
    
    void reset_program();
        
    const QString default_brainfuck = 
            "++++++++[>++++[>++>+++>+++>+<<<<-]"
            ">+>->+>>+[<]<-]>>.>>---.+++++++..+"
            "++.>.<<-.>.+++.------.--------.>+.>++.";
    
    QString program = "";
    
    volatile bool stop = false; //To be assigned from other thread to halt program
    bool running = false;
    
    std::vector<char> memory;
    uint64_t mem_index = 0;
    
    bool update_ui = false;
    ui_updates_struct ui_updates;
    
    QString input = "";
    QString output = "";
    QString error_message = "";
    
    bool max_instructions_enforced = false;
    uint max_instructions = 0;
    
    bool max_mem_enforced = false;
    uint64_t max_memory = 0;
    
    uint64_t memory_access_count = 0;
    uint64_t instruction_count = 0;
    
    uint64_t execution_time = 0; //ms
    
signals:
    
    void requestUIUpdate(ui_updates_struct updates);
    
    void programExit(int error);
    
    
public slots:
    
    void runProgram();
    
    
    
};

#endif // BRAINFUCK_H
