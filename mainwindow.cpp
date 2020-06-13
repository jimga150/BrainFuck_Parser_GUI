#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    //TODO: add optional delay between instructions
    //TODO: add ability to pause program and resume (separate from killing it)
    
    this->ui->Program_textbox->setText(this->brainfuck.program);
    
    this->ui->maxMem_spinBox->setMaximum(INT_MAX);
    this->ui->maxInst_spinBox->setMaximum(INT_MAX);
    
    this->ui->maxMem_spinBox->setValue(this->brainfuck.max_memory);
    this->ui->maxInst_spinBox->setValue(this->brainfuck.max_instructions);
    
    QFont output_font("Monaco");
    output_font.setStyleHint(QFont::Courier);
    this->ui->Output->setFont(output_font);
    
    this->ui->Output->setLineWrapMode(QTextEdit::WidgetWidth);
    this->on_textWrapping_checkBox_stateChanged(static_cast<int>(this->ui->textWrapping_checkBox->checkState()));
    
    connect(&this->brainfuck, &BrainFuck::requestUIUpdate, this, &MainWindow::updateUI);
    connect(&this->brainfuck, &BrainFuck::programExit, this, &MainWindow::programFinished);
}

MainWindow::~MainWindow()
{
    if (this->output_file) delete this->output_file;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event){
    Q_UNUSED(event)
    
    //ask BF to stop
    this->brainfuck.stop = true;
    connect(&(this->brainfuck), &BrainFuck::programExit, this, &QMainWindow::close);
}

void MainWindow::updateUI(ui_updates_struct updates){
    if (updates.update_output){
        this->update_output();
    }
    if (updates.update_mem || updates.update_mem_ptr){ //TODO: separate these?
        this->update_memDisplay();
    }
    QApplication::processEvents();
}

void MainWindow::update_output(){
    this->ui->Output->setText(this->brainfuck.output);
    
    QFontMetrics fm(this->ui->Output->currentFont());
    QSize text_size(fm.size(Qt::TextExpandTabs, this->brainfuck.output));
    
    QSize viewport = this->ui->Output->geometry().size();
    
    bool auto_zoom = this->ui->textWrapping_checkBox->checkState() == Qt::Unchecked;
    
    if (auto_zoom && (viewport.width() < text_size.width() || viewport.height() < text_size.height())){
        this->ui->Output->zoomOut();
    }
}

void MainWindow::update_memDisplay(){
    
    int min_cell_width = 100; //pixels //TODO: magic number
    
    QGridLayout* layout = static_cast<QGridLayout*>(this->ui->memDisplay->layout());
    
    int widget_width = this->ui->memDisplay->width();
    int num_cells = widget_width/min_cell_width;
    uint64_t start_mem_index = this->brainfuck.mem_index - this->brainfuck.mem_index % num_cells;
    
    //TODO: add separate case for just shifting memory frame rather than redoing whole UI portion
    if (widget_width != this->memCellUIs.last_widget_width || start_mem_index != this->memCellUIs.last_start_mem_index){
        
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
    }
    
    for (int i = 0; i < num_cells; ++i){
        
        uint mem_index = start_mem_index + i;
        this->memCellUIs.labels[i].setNum(static_cast<int>(mem_index));
        
        int mem_value = mem_index < this->brainfuck.memory.size() ? this->brainfuck.memory[mem_index] : 0;
        this->memCellUIs.cells[i].setText(QString::number(mem_value));
    }
    
    if (this->brainfuck.mem_index != this->memCellUIs.last_pointer_index){
        this->memCellUIs.pointer_row[this->memCellUIs.last_pointer_index % num_cells].setText("");
        this->memCellUIs.pointer_row[this->brainfuck.mem_index % num_cells].setText("^");
        this->memCellUIs.last_pointer_index = this->brainfuck.mem_index;
    }
}

void MainWindow::programFinished(int errorCode){
    
    double mem_access_percent = (this->brainfuck.memory_access_count*100.0)/(this->brainfuck.instruction_count);
    
    this->ui->Console->append(
                "Program finished with exit code " + QString::number(errorCode) + 
                "\n" + (errorCode ? "Error: " + this->brainfuck.error_message + "\n" : "") + 
                "\nMemory used: " + QString::number(this->brainfuck.memory.size()) + " Bytes" +            
                "\nInstructions used: " + QString::number(this->brainfuck.instruction_count) + 
                "\nMemory accesses: " + QString::number(this->brainfuck.memory_access_count) + 
                "\n" + QString::number(mem_access_percent, 'g', 3) + "% of instructions accessed memory" + 
                (this->brainfuck.execution_time == 0 ? "" : "\nExecution time: " + QString::number(this->brainfuck.execution_time) + " ms")
                );
    
    this->update_output();
    
    if (this->output_file){
        this->output_file->write(this->brainfuck.output.toUtf8().constData());
        this->output_file->flush();
    }
    
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
        //TODO: notify user
        return;
    }
    
    QTextStream stream(&infile);
    
    this->brainfuck.program = stream.readAll();
    
    this->ui->Program_textbox->setText(this->brainfuck.program);
}

void MainWindow::on_Program_textbox_textChanged(){
    this->brainfuck.program = this->ui->Program_textbox->toPlainText();
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
        //TODO: notify user
        return;
    }
    
    QTextStream stream(&infile);
    
    this->brainfuck.input = stream.readAll();
    
    this->ui->Input->setText(this->brainfuck.input);
}

void MainWindow::on_Input_textChanged(){
    this->brainfuck.input = this->ui->Input->toPlainText();
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
    
    this->brainfuck.max_memory = coeff*multiplier;
}

void MainWindow::on_maxInst_spinBox_valueChanged(int arg1){
    this->brainfuck.max_instructions = arg1;
}

void MainWindow::on_start_pause_button_clicked(){
    if (this->brainfuck.running){
        this->brainfuck.stop = true;
        this->ui->start_pause_button->setText("Fuck it!!! (execute)"); //TODO: derive these strings from the same place
    } else {
        this->brainfuck.reset_program();
        this->ui->Console->clear();
        QTimer::singleShot(100, &(this->brainfuck), SLOT(runProgram()));
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
    
    this->brainfuck.max_instructions_enforced = enforce_inst;
    
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
    
    this->brainfuck.max_mem_enforced = enforce_mem;
    
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
