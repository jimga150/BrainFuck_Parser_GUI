#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

struct brainfuck_struct{
    QString program = "";
    
    std::vector<char> memory;
    uint64_t mem_index = 0;
    
    QString input = "";
    QString output = "";
    
    uint instructions_per_ui_update = 1;
    uint max_instructions = INT_MAX;
    uint max_memory = INT_MAX;
    
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
    
    void updateUI();
        
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
    
    void on_start_button_clicked();
    
    void on_Input_textChanged();
    
    
    void on_Program_textbox_textChanged();
    
private:
    Ui::MainWindow *ui;
    
};
#endif // MAINWINDOW_H
