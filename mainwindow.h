#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

#include "brainfuck.h"
#include "ui_mainwindow.h"


#define BYTE_BASE (1)
#define KB_BASE (1024)
#define MB_BASE (KB_BASE*KB_BASE)
#define GB_BASE (KB_BASE*KB_BASE*KB_BASE)


struct memory_cell_displays_struct{
    
    QLabel* labels = nullptr;
    QPushButton* cells = nullptr;
    QLabel* pointer_row = nullptr;
    
    int num_cells = 0;
    
    uint64_t last_pointer_index = 0;
    
    memory_cell_displays_struct(){}
    
    memory_cell_displays_struct(int num_cells){
        this->labels = new QLabel[num_cells];
        this->cells = new QPushButton[num_cells];
        this->pointer_row = new QLabel[num_cells];
        this->pointer_row[0].setText("^");
        
        this->num_cells = num_cells;
    }
    
    ~memory_cell_displays_struct(){
        if (this->labels) delete [] this->labels;
        if (this->cells) delete [] this->cells;
        if (this->pointer_row) delete [] this->pointer_row;
    }
    
    void change_num_cells(int new_numcells){
        QLabel* new_labels = new QLabel[new_numcells];
        QPushButton* new_cells = new QPushButton[new_numcells];
        QLabel* new_pointer_row = new QLabel[new_numcells];
        
        for (int i = 0; i < qMin(new_numcells, this->num_cells); ++i){
            if (this->labels) new_labels[i].setText(labels[i].text());
            if (this->cells) new_cells[i].setText(cells[i].text());
            if (this->pointer_row) new_pointer_row[i].setText(pointer_row[i].text());
        }
        
        this->~memory_cell_displays_struct();
        
        this->labels = new_labels;
        this->cells = new_cells;
        this->pointer_row = new_pointer_row;
        this->num_cells = new_numcells;
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
    
    void update_output();
    
    void update_memDisplay();
    
    
    BrainFuck brainfuck;
    
    QFile* output_file = nullptr;
    
    memory_cell_displays_struct memCellUIs;
    
    
public slots:
    
    void updateUI(ui_updates_struct updates);
    
    void programFinished(int errorCode);
    
    
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
    
    void on_textWrapping_checkBox_stateChanged(int arg1);
    
private:
    Ui::MainWindow *ui;
    
};
#endif // MAINWINDOW_H
