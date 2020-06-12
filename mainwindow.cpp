#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    //TODO: add optional text wrapping to Output
    this->ui->Program_textbox->setText(this->brainfuck.program);
    
    this->ui->maxMem_spinBox->setMaximum(INT_MAX);
    this->ui->maxInst_spinBox->setMaximum(INT_MAX);
    
    this->ui->maxMem_spinBox->setValue(this->brainfuck.max_memory);
    this->ui->maxInst_spinBox->setValue(this->brainfuck.max_instructions);
    
    QFont output_font("Monaco");
    output_font.setStyleHint(QFont::Courier);
    this->ui->Output->setFont(output_font);
    
    connect(&this->brainfuck, &BrainFuck::requestUIUpdate, this, &MainWindow::updateUI);
    connect(&this->brainfuck, &BrainFuck::programExit, this, &MainWindow::programFinished);
}

MainWindow::~MainWindow()
{
    if (this->output_file) delete this->output_file;
    delete ui;
}


void MainWindow::updateUI(){
    //TODO: have brainfuck send struct of bools representing things to update
    if (this->brainfuck.update_output){
        this->update_output();
        this->brainfuck.update_output = false;
    }
    
    QApplication::processEvents();
}

void MainWindow::update_output(){
    this->ui->Output->setText(this->brainfuck.output);
    
    QFontMetrics fm(this->ui->Output->currentFont());
    QSize text_size(fm.size(Qt::TextExpandTabs, this->brainfuck.output));
    
    QSize viewport = this->ui->Output->geometry().size();
    
    if (viewport.width() < text_size.width() || viewport.height() < text_size.height()){
        this->ui->Output->zoomOut();
    }
}

void MainWindow::programFinished(int errorCode){
    
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
        QTimer::singleShot(100, &(this->brainfuck), SLOT(runProgram()));
        this->ui->start_pause_button->setText("Pause Program");
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
