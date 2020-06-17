#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    //TODO: add ability to pause program and resume (separate from killing it)
    
    this->monitor_refresh_rate_ms = static_cast<int>(1000.0/this->screen()->refreshRate());
    
    this->ui->Program_textbox->setText(this->brainfuck.getProgram());
    
    this->ui->maxMem_spinBox->setMaximum(INT_MAX);
    this->ui->maxInst_spinBox->setMaximum(INT_MAX);
    
    this->ui->maxMem_spinBox->setValue(this->brainfuck.getMaxMem());
    this->ui->maxInst_spinBox->setValue(this->brainfuck.getMaxInstructions());
    
    QFont output_font("Monaco");
    output_font.setStyleHint(QFont::Courier);
    this->ui->Output->setFont(output_font);
    
    this->ui->Output->setLineWrapMode(QTextEdit::WidgetWidth);
    this->on_textWrapping_checkBox_stateChanged(static_cast<int>(this->ui->textWrapping_checkBox->checkState()));
    
    //allows ui_updates_struct to be used in signals and slots
    qRegisterMetaType<ui_updates_struct>();
    
    connect(&this->brainfuck, &BrainFuck::requestUIUpdate, this, &MainWindow::updateUIPartial, Qt::BlockingQueuedConnection); 
    this->program_exit_connection = connect(&this->program_thread_watcher, &QFutureWatcher<program_post_struct>::finished, this, &MainWindow::programFinished);
}

MainWindow::~MainWindow()
{
    if (this->output_file) delete this->output_file;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event){
    Q_UNUSED(event)
    
    //ask BF to stop
    if (this->brainfuck.running){
        this->brainfuck.stop = true;
        disconnect(this->program_exit_connection);
        this->program_thread.result();
    }
}

void MainWindow::showEvent(QShowEvent* event){
    Q_UNUSED(event)
    
    this->updateUI(false);
}

void MainWindow::reset_UI(){
    this->ui->Output->clear();
    this->ui->Console->clear();
    this->ui->Input->setText(this->brainfuck.getInput());
    this->ui->Program_textbox->setText(this->brainfuck.getProgram());
}

void MainWindow::updateUI(bool force){
    
    ui_updates_struct ui_state;
    bool can_update = this->brainfuck.getCurrentState(&ui_state);
    
    if (!can_update) return;
    
    this->update_output(ui_state.output);
    
    int num_cells = updateMemCellLayout(ui_state.mem_ptr);
    this->update_memPtr(ui_state.mem_ptr, num_cells);
    this->update_memVal(ui_state.memory, num_cells);
    
    if (force){
        QApplication::processEvents();
    }
}

void MainWindow::updateUIPartial(ui_updates_struct updates){
    
    //printf("Partial update requested: %llu\n", this->ui_update_timer.elapsed());
    
    this->pending_updates |= updates;
    
    if (!this->ui_update_timer.isValid()){
        this->ui_update_timer.start();
        return;
    }
    
    if (this->ui_update_timer.elapsed() < this->monitor_refresh_rate_ms){
//        if (updates.update_output){
//            printf("Output update set aside for later: %s\n", updates.output.toUtf8().constData());
//        }
        return;
    }
    
    //printf("Timer high enough, continuing... (%llu)\n", updates.mem_ptr);
    
    this->ui_update_timer.restart();
    
    bool anyUpdates = false;
    
    if (this->pending_updates.update_output){
        anyUpdates = true;
        this->update_output(pending_updates.output);
    }
    if (this->pending_updates.update_mem || updates.update_mem_ptr){
        anyUpdates = true;
        int numcells = this->updateMemCellLayout(pending_updates.mem_ptr);
        
        if (this->pending_updates.update_mem_ptr){
            this->update_memPtr(this->pending_updates.mem_ptr, numcells);
        }
        if (this->pending_updates.update_mem){
            this->update_memVal(this->pending_updates.memory, numcells);
        }
    }
    if (anyUpdates){
        QApplication::processEvents();
        this->pending_updates.reset();
    }
}

void MainWindow::update_output(QString new_output){
    
    //printf("Updating output: %s\n", new_output.toUtf8().constData());
    
    this->ui->Output->setText(new_output);
    
    QFontMetrics fm(this->ui->Output->currentFont());
    QSize text_size(fm.size(Qt::TextExpandTabs, new_output));
    
    QSize viewport = this->ui->Output->geometry().size();
    
    bool auto_zoom = this->ui->textWrapping_checkBox->checkState() == Qt::Unchecked;
    
    if (auto_zoom && (viewport.width() < text_size.width() || viewport.height() < text_size.height())){
        this->ui->Output->zoomOut();
    }
}

void MainWindow::update_memPtr(uint64_t current_mem_index, int num_cells){    
    if (current_mem_index != this->memCellUIs.last_pointer_index){
        this->memCellUIs.pointer_row[this->memCellUIs.last_pointer_index % num_cells].clear();
        this->memCellUIs.pointer_row[current_mem_index % num_cells].setText(this->pointer_label);
        this->memCellUIs.last_pointer_index = current_mem_index;
    }
}

void MainWindow::update_memVal(QVector<char> memory, int num_cells){
    for (int i = 0; i < num_cells; ++i){
        uint mem_index = this->memCellUIs.last_start_mem_index + i;
        int mem_value = mem_index < static_cast<uint64_t>(memory.size()) ? memory[mem_index] : 0;
        this->memCellUIs.cells[i].setText(QString::number(mem_value, 16).toUpper().right(2));
    }
}

int MainWindow::updateMemCellLayout(uint64_t current_mem_index){
    int min_cell_width = 30; //pixels //TODO: magic number
    
    QGridLayout* layout = static_cast<QGridLayout*>(this->ui->memDisplay->layout());
    
    int widget_width = this->ui->memDisplay->width();
    //printf("%d\n", widget_width);
    //fflush(stdout);
    int num_cells = widget_width/min_cell_width;
    uint64_t start_mem_index = current_mem_index - current_mem_index % num_cells;
    
    //TODO: add separate case for just shifting memory frame rather than redoing whole UI portion
    if (widget_width != this->memCellUIs.last_widget_width || start_mem_index != this->memCellUIs.last_start_mem_index){
        
        //printf("New widget width: %d; num cells: %d\n", widget_width, num_cells);
        //fflush(stdout);
        
        //delete existing layout items (widgets inside will be preserved)
        QLayoutItem* item = nullptr;
        while((item = layout->takeAt(0))){
            //Dont delete widget, we own that and manage it elsewhere
            delete item;
        }
        
        this->memCellUIs.change_num_cells(num_cells);
        
        for (int i = 0; i < num_cells; ++i){        
            layout->addWidget(&(this->memCellUIs.labels[i]), 0, i, Qt::AlignHCenter);
            layout->addWidget(&(this->memCellUIs.cells[i]), 1, i, Qt::AlignHCenter);
            layout->addWidget(&(this->memCellUIs.pointer_row[i]), 2, i, Qt::AlignHCenter);
        }
        
        for (int i = 0; i < num_cells; ++i){
            uint mem_index = start_mem_index + i;
            this->memCellUIs.labels[i].setNum(static_cast<int>(mem_index));
        }
        
        if (this->memCellUIs.last_widget_width == 0){
            this->memCellUIs.pointer_row[0].setText(this->pointer_label);
        }
        
        this->memCellUIs.last_widget_width = widget_width;
        this->memCellUIs.last_start_mem_index = start_mem_index;
    }
    
    return num_cells;
}

void MainWindow::programFinished(){
    
    program_post_struct exit_info = this->program_thread.result();
    
    double mem_access_percent = (exit_info.memory_access_count*100.0)/(exit_info.instruction_count);
    
    unsigned long long exec_time = exit_info.execution_time;
    double ips = exit_info.instruction_count*1.0/(exec_time/1000.0);
    
    this->ui->Console->append(
                "Program finished with exit code " + QString::number(exit_info.error_code) + 
                "\n" + (exit_info.error_code ? "Error: " + exit_info.error_message + "\n" : "") + 
                "\nMemory used: " + QString::number(exit_info.memory_size) + " Bytes" +            
                "\nInstructions used: " + QString::number(exit_info.instruction_count) + 
                "\nMemory accesses: " + QString::number(exit_info.memory_access_count) + 
                "\n" + QString::number(mem_access_percent, 'g', 3) + "% of instructions accessed memory" + 
                (exec_time == 0 ? "" : "\nExecution time: " + QString::number(exec_time) + " ms") + 
                (exec_time == 0 ? "" : "\nInstructions per second: " + QString::number(ips))
                );
    
    this->updateUI(false);
    
    if (this->output_file){
        this->output_file->write(exit_info.output.toUtf8().constData());
        this->output_file->flush();
    }
    
    this->ui->Program_textbox->setEnabled(true);
    this->ui->progFile_button->setEnabled(true);
    this->ui->start_pause_button->setText("Fuck it!!! (execute)"); //TODO: derive these strings from the same place
}

void MainWindow::on_progFile_button_clicked(){
    QString filename = QFileDialog::getOpenFileName(
                           this, 
                           "Select program file", 
                           QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0)
                           );
    
    if (filename.isNull()){
        return;
    }
    
    QFile infile(filename);
    if (!infile.open(QIODevice::ReadOnly | QIODevice::Text)){
        fprintf(stderr, "Cannot open program file at %s\n", filename.toUtf8().constData());
        return;
    }
    
    QTextStream stream(&infile);
    
    QString new_prog = stream.readAll();
    
    if (this->brainfuck.setProgram(new_prog)){
        this->ui->Program_textbox->setText(new_prog);
    }
}

void MainWindow::on_Program_textbox_textChanged(){
    this->brainfuck.setProgram(this->ui->Program_textbox->toPlainText());
}

void MainWindow::on_inFile_button_clicked(){
    
    QString filename = QFileDialog::getOpenFileName(
                           this, 
                           "Select input file", 
                           QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0)
                           );
    
    if (filename.isNull()){
        return;
    }
    
    QFile infile(filename);
    if (!infile.open(QIODevice::ReadOnly | QIODevice::Text)){
        fprintf(stderr, "Cannot open input file at %s\n", filename.toUtf8().constData());
        return;
    }
    
    QTextStream stream(&infile);
    
    QString new_input = stream.readAll();
    
    if (this->brainfuck.setInput(new_input)){
        this->ui->Input->setText(new_input);
    } 
}

void MainWindow::on_Input_textChanged(){
    //TODO: find way of making this work when program is running
    this->brainfuck.setInput(this->ui->Input->toPlainText());
}

void MainWindow::on_outFile_button_clicked(){
    if (this->output_file){
        this->output_file->close();
    }
    
    QString filename = QFileDialog::getOpenFileName(
                           this, 
                           "Select input file", 
                           QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0)
                           );
    
    if (filename.isNull()){
        return;
    }
    
    this->output_file = new QFile(filename);
    if (!this->output_file->open(QIODevice::WriteOnly | QIODevice::Truncate)){
        //TODO: alert user
        delete this->output_file;
        this->output_file = nullptr;
        return;
    }
}

void MainWindow::on_maxMem_spinBox_valueChanged(int arg1){
    Q_UNUSED(arg1)
    this->update_maxmem();
}

void MainWindow::on_max_mem_units_comboBox_currentIndexChanged(const QString &arg1){
    Q_UNUSED(arg1)
    this->update_maxmem();
}

void MainWindow::update_maxmem(){
    uint64_t multiplier = 1;
    
    QString units = this->ui->max_mem_units_comboBox->currentText();
    switch(units.toUtf8().constData()[0]){
    case 'B':
        multiplier = BYTE_BASE;
        break;
    case 'K':
        multiplier = KB_BASE;
        break;
    case 'M':
        multiplier = MB_BASE;
        break;
    case 'G':
        multiplier = GB_BASE;
        break;
    default:
        fprintf(stderr, "Error: Unit not found: %s\n", units.toUtf8().constData());
        break;
    }
    
    uint coeff = this->ui->maxMem_spinBox->value();
    
    this->brainfuck.setMaxMem(coeff*multiplier);
}

void MainWindow::on_maxInst_spinBox_valueChanged(int arg1){
    this->brainfuck.setMaxInstructions(arg1);
}

void MainWindow::on_start_pause_button_clicked(){
    if (this->brainfuck.running){
        
        this->brainfuck.stop = true;
        
    } else {
        
        this->brainfuck.reset_program();
        
        this->reset_UI();
        this->ui->Program_textbox->setEnabled(false);
        this->ui->progFile_button->setEnabled(false);
        
        this->program_thread = QtConcurrent::run(&(this->brainfuck), &BrainFuck::runProgram);
        this->program_thread_watcher.setFuture(this->program_thread);
        
        this->ui->start_pause_button->setText("Halt Program");
    }
}

void MainWindow::on_zoom_in_output_button_clicked(){
    this->ui->Output->zoomIn();
}

void MainWindow::on_zoom_out_output_button_clicked(){
    this->ui->Output->zoomOut();
}


void MainWindow::on_limit_inst_checkBox_stateChanged(int arg1){
    
    bool enforce_inst;
    
    Qt::CheckState state = static_cast<Qt::CheckState>(arg1);
    switch(state){
    case Qt::Unchecked:
        enforce_inst = false;
        break;
    case Qt::Checked:
    case Qt::PartiallyChecked:
        enforce_inst = true;
        break;
    default:
        enforce_inst = false;
        break;
    }
    
    this->brainfuck.setMaxInstEnforced(enforce_inst);
    
    this->ui->max_inst_label->setEnabled(enforce_inst);
    this->ui->maxInst_spinBox->setEnabled(enforce_inst);
    
    if (enforce_inst){
        this->on_maxInst_spinBox_valueChanged(this->ui->maxInst_spinBox->value());
    }
}

void MainWindow::on_max_mem_checkBox_stateChanged(int arg1){
    
    bool enforce_mem;
    
    Qt::CheckState state = static_cast<Qt::CheckState>(arg1);
    switch(state){
    case Qt::Unchecked:
        enforce_mem = false;
        break;
    case Qt::Checked:
    case Qt::PartiallyChecked:
        enforce_mem = true;
        break;
    default:
        enforce_mem = false;
        break;
    }
    
    this->brainfuck.setMaxMemEnforced(enforce_mem);
    
    this->ui->max_mem_label->setEnabled(enforce_mem);
    this->ui->maxMem_spinBox->setEnabled(enforce_mem);
    this->ui->max_mem_units_comboBox->setEnabled(enforce_mem);
    
    if (enforce_mem){
        this->update_maxmem();
    }
}

void MainWindow::on_textWrapping_checkBox_stateChanged(int arg1){
    
    bool enable_wrapping;
    
    Qt::CheckState state = static_cast<Qt::CheckState>(arg1);
    switch(state){
    case Qt::Unchecked:
        enable_wrapping = false;
        break;
    case Qt::Checked:
    case Qt::PartiallyChecked:
        enable_wrapping = true;
        break;
    default:
        enable_wrapping = false;
        break;
    }
    
    if (enable_wrapping){
        this->ui->Output->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    } else {
        this->ui->Output->setWordWrapMode(QTextOption::NoWrap);
    }
}

void MainWindow::on_timeDelay_horizontalSlider_sliderMoved(int position){
    this->ui->timeDelay_spinBox->setValue(position);
    this->brainfuck.command_delay = position;
}

void MainWindow::on_timeDelay_spinBox_editingFinished(){
    int new_value = this->ui->timeDelay_spinBox->value();
    this->ui->timeDelay_horizontalSlider->setValue(new_value);
    this->brainfuck.command_delay = new_value;
}
