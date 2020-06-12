#ifndef BRAINFUCK_H
#define BRAINFUCK_H

#include <QObject>
#include <QTextStream>

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
    
    volatile bool stop = false;
    bool running = false;
    
    std::vector<char> memory;
    uint64_t mem_index = 0;
    
    //TODO: probably an input pointer i can display
    bool update_mem_ptr = false;
    bool update_mem = false;
    bool update_output = false;
    
    bool update_some_ui = false;
    
    QString input = "";
    QString output = "";
    
    bool max_instructions_enforced = false;
    uint max_instructions = INT_MAX;
    
    bool max_mem_enforced = false;
    uint64_t max_memory = ULONG_LONG_MAX;
    
    uint64_t memory_access_count = 0;
    uint64_t instruction_count = 0;
    
signals:
    
    void requestUIUpdate();
    
    void programExit(int error);
    
    
public slots:
    
    void runProgram();
    
    
    
};

#endif // BRAINFUCK_H
