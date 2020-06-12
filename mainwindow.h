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
    
        
    BrainFuck brainfuck;
    
    QFile* output_file = nullptr;
    
    
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
    
private:
    Ui::MainWindow *ui;
    
};
#endif // MAINWINDOW_H
