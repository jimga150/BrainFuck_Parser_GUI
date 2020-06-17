#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QScreen>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>
#include <QtConcurrent>

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
    int last_widget_width = 0;
    uint64_t last_start_mem_index = 0;
    
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
    
    void closeEvent(QCloseEvent* event) override;
    
    void showEvent(QShowEvent* event) override;
    
    void update_maxmem();
    
    void update_output(QString new_output);
    
    void update_memDisplay(uint64_t mem_index, QVector<char> memory);
    
    void reset_UI();
    
    
    const QString pointer_label = "^";
    
    
    BrainFuck brainfuck;
    
    QFile* output_file = nullptr;
    
    memory_cell_displays_struct memCellUIs;
    
    QElapsedTimer ui_update_timer;
    int monitor_refresh_rate_ms = 0;
    ui_updates_struct pending_updates;
    
    QMetaObject::Connection program_exit_connection;
    QFutureWatcher<program_post_struct> program_thread_watcher;
    QFuture<program_post_struct> program_thread;
    
    
public slots:
    
    void updateUI(bool force);
    
    void updateUIPartial(ui_updates_struct updates);
    
    void programFinished();
    
    
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
    
    void on_timeDelay_horizontalSlider_sliderMoved(int position);
        
    void on_timeDelay_spinBox_editingFinished();
    
private:
    Ui::MainWindow *ui;
    
};
#endif // MAINWINDOW_H
