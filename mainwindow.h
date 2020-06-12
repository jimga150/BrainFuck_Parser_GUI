#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

#define BYTE_BASE (1)
#define KB_BASE (1024)
#define MB_BASE (KB_BASE*KB_BASE)
#define GB_BASE (KB_BASE*KB_BASE*KB_BASE)

struct brainfuck_struct{
    QString program = "";
    bool stop = false;
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
    
    brainfuck_struct(){
        this->memory.push_back(0);
    }
    
    void reset_program(){
        this->memory.clear();
        this->memory.push_back(0);
        this->mem_index = 0;
        this->output = "";
        this->memory_access_count = 0;
        this->instruction_count = 0;
    }
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
        
    void update_maxmem();
        
    const QString default_brainfuck = 
            "++++++++[>++++[>++>+++>+++>+<<<<-]"
            ">+>->+>>+[<]<-]>>.>>---.+++++++..+"
            "++.>.<<-.>.+++.------.--------.>+.>++.";
    
    brainfuck_struct bf_header;
    
    QFile* output_file = nullptr;
    
    
public slots:
    
    void runProgram();
    
    
private slots:
    
    void on_maxMem_spinBox_valueChanged(int arg1);
    
    void on_maxInst_spinBox_valueChanged(int arg1);
    
    void on_inFile_button_clicked();
    
    void on_outFile_button_clicked();
    
    void on_start_pause_button_clicked();
    
    void on_Input_textChanged();
    
    void on_Program_textbox_textChanged();
    
    void on_progFile_button_clicked();
        
    void on_zoom_in_output_button_clicked();
    
    void on_zoom_out_output_button_clicked();
    
    void on_max_mem_units_comboBox_currentIndexChanged(const QString &arg1);
    
    void on_limit_inst_checkBox_stateChanged(int arg1);
    
    void on_max_mem_checkBox_stateChanged(int arg1);
    
private:
    Ui::MainWindow *ui;
    
};
#endif // MAINWINDOW_H
