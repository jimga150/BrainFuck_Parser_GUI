#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    this->bf_header.program = this->default_brainfuck;
    this->ui->Program_textbox->setText(this->bf_header.program);
    
    this->ui->maxMem_spinBox->setMaximum(INT_MAX);
    this->ui->maxInst_spinBox->setMaximum(INT_MAX);
    
    this->ui->maxMem_spinBox->setValue(this->bf_header.max_memory);
    this->ui->maxInst_spinBox->setValue(this->bf_header.max_instructions);
    
    QFont output_font("Monaco");
    output_font.setStyleHint(QFont::Courier);
    this->ui->Output->setFont(output_font);
    //this->ui->Output->setCurrentFont(output_font);
}

MainWindow::~MainWindow()
{
    if (this->output_file) delete this->output_file;
    delete ui;
}

void MainWindow::runProgram(){
    
    this->bf_header.running = true;
    this->ui->start_pause_button->setText("Pause Program");
    
    const QString valid_commands = "><+-[].,";
    
    QTextStream input(&(this->bf_header.input), QIODevice::ReadOnly);
                
//    for (QChar c : this->bf_header.program){
//        bool valid = false;
//        for (QChar v : valid_commands){
//            if (v == c){
//                valid = true;
//                break;
//            }
//        }
//        if (!valid){
//            this->bf_header.program.remove(c); //TODO: test this
//        }
//    }
    
    uint program_length = this->bf_header.program.length();
    char program_chars[program_length];
    for (uint i = 0; i < program_length; ++i){
        program_chars[i] = this->bf_header.program.toUtf8().constData()[i];
    }
    
//    printf("QString: \t%s\n", this->bf_header.program.toUtf8().constData());
//    printf("char*: \t%s\n", program_chars);
            
    //build loop list
    long LUT[program_length];
    //printf("%p\n", (void*)LUT);
    int levels[program_length];
    
    int looplevel = 0;
    for(uint i = 0; i < program_length; ++i){
        //printf("Loop level: %d\n", looplevel);
        switch(program_chars[i]){
        case '[':
            levels[i] = ++looplevel;
            break;
        case ']':
            levels[i] = -1*(looplevel--);
            break;
        default:
            levels[i] = 0;
            break;
        }
    }
    
//    printf("Levels: [");
//    for (uint i = 0; i < program_length; ++i){
//        printf("%d, ", levels[i]);
//    }
//    printf("]\n");
        
    if (looplevel){
        //bracket mismatch
        fprintf(stderr, "Bracket mismatch (last known level: %d, exiting)...\n", looplevel);
        QCoreApplication::exit(1);
    }
    
    //make lookup table for brackets based on loop level list
    //Opening brackets get index of their closing counterpart, and vice-versa
    for(uint i = 0; i < program_length; ++i){
        
        if (levels[i] > 0){
            
            //printf("Found opening bracket: level %ld, character %d\n", LUT[i], i);
            int matching_bracket_id = -1*levels[i];
            
            //printf("finding %d\n", matching_bracket_id);
            for (ulong b=(ulong)i; b < program_length; ++b){
                
                //printf("b loop: b=%lu\n", b);
                if (levels[b] == matching_bracket_id){
                    
                    //printf("Found matching closer at position %lu\n", b);
                    LUT[i] = b;
                    LUT[b] = (long)i;
                    break;
                }
            }
        }
    }
    
    //printf("printing LUT:\n");
    //printLongArray(stdout, LUT, char_count, "%ld ");
    
    //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");	
    
    bool halt = false;
        
//#if FALSE //#ifdef RAND_MEM
//    printf("Randomizing memory...\n");
//    //Loop through and randomize all memory
//    for (int i=0; i < memory_size; i++){
//        char rand_num = rand() % (UCHAR_MAX+1);
//        //printf("Setting index %d to %u\n", i, (int)((unsigned char)rand_num));
//        memory[i] = rand_num;
//        //printf("Memory at %d is %x\n", i, ptr[i]);
//    }
//#endif
    
    //printf("Made it to program loop\n");
    for(
        uint64_t i = 0; 
        
        !this->bf_header.stop && !halt && 
        i < program_length && 
        (!this->bf_header.max_mem_enforced || (this->bf_header.instruction_count != this->bf_header.max_instructions)); 
        
        i++){
        
        ++this->bf_header.instruction_count;
        
//        printf("%llu: %llu-> %c (%d), mem ptr @ %llu, memory: [", this->bf_header.instruction_count, i, program_chars[i], (int)program_chars[i], this->bf_header.mem_index);
//        for (char c : this->bf_header.memory){
//            printf("%d, ", (int)c);
//        }
//        printf("]\n");
        
        switch (program_chars[i]){
        case '>':
            
            ++this->bf_header.mem_index;
            
            if (this->bf_header.max_mem_enforced && (this->bf_header.mem_index == this->bf_header.max_memory)){
                halt = true;
                break;
            }
            
            if (this->bf_header.mem_index >= this->bf_header.memory.size()){
                this->bf_header.memory.push_back(0);
            }
            
            break;
        case '<':

            if (this->bf_header.mem_index-- == 0){
                //TODO: make useful error message channel
                fprintf(stderr, "BrainFucked!!! (memory index underflow, exiting)\n");
                halt = true;
            }
            
            break;
        case '+':
            
            ++this->bf_header.memory_access_count;
            ++this->bf_header.memory[this->bf_header.mem_index];
            
            break;
        case '-':
            
            ++this->bf_header.memory_access_count;
            --this->bf_header.memory[this->bf_header.mem_index];
            
            break;
        case '['://if pointer is 0, jump to command AFTER ending bracket
            
            ++this->bf_header.memory_access_count;
            
            if (!(this->bf_header.memory[this->bf_header.mem_index])){
                i = LUT[i];
            }
            
            break;
        case ']'://jump to complimentary opening bracket if pointer is non-zero
            
            ++this->bf_header.memory_access_count;
            
            if (this->bf_header.memory[this->bf_header.mem_index]){
                i = LUT[i];
            }
            
            break;
        case '.':
            
            ++this->bf_header.memory_access_count;
            
            this->bf_header.output.append(this->bf_header.memory[this->bf_header.mem_index]);
            
            this->bf_header.update_output = true;
            this->bf_header.update_some_ui = true;
            
            break;
        case ',':
            
            ++this->bf_header.memory_access_count;
            
            if (input.atEnd()){
                fprintf(stderr, "End reached in input stream, exiting...\n");
                halt = true;
                break;
            }
            
            input >> this->bf_header.memory[this->bf_header.mem_index];
            
            break;
        default:
            //ignore any other input
            break;
        }
        
        if (this->bf_header.update_some_ui){
            
            if (this->bf_header.update_output){
                this->ui->Output->setText(this->bf_header.output);
                
                QFontMetrics fm(this->ui->Output->currentFont());
                QSize text_size(fm.size(Qt::TextExpandTabs, this->bf_header.output));
                
                QSize viewport = this->ui->Output->geometry().size();
                
                if (viewport.width() < text_size.width() || viewport.height() < text_size.height()){
                    this->ui->Output->zoomOut();
                }
                
                this->bf_header.update_output = false;
            }
            
            QApplication::processEvents();
            this->bf_header.update_some_ui = false;
        }
    }
        
    //printf("%s\n", this->bf_header.output.toUtf8().constData());    
    //printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    
//    fprintf(stdout, "Memory used: %ld\n", this->bf_header.memory.size());
//    fprintf(stdout, "# instructions: %llu\n", this->bf_header.instruction_count);
//    fprintf(stdout, "# of memory accesses: %llu\n", this->bf_header.memory_access_count);
    
//    double mem_access_percent = ((double)this->bf_header.memory_access_count*100)/((double)this->bf_header.instruction_count);
//    fprintf(stdout, "%% of instructions that access memory: %0.1f%%\n", mem_access_percent);
    
    if (this->output_file){
        this->output_file->write(this->bf_header.output.toUtf8().constData());
        this->output_file->flush();
    }
    
    this->ui->start_pause_button->setText("Fuck it!!! (execute)"); //TODO: derive these strings from the same place
    this->bf_header.running = false;
    this->bf_header.stop = false;
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
    
    this->bf_header.program = stream.readAll();
    
    this->ui->Program_textbox->setText(this->bf_header.program);
}

void MainWindow::on_Program_textbox_textChanged(){
    this->bf_header.program = this->ui->Program_textbox->toPlainText();
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
    
    this->bf_header.input = stream.readAll();
    
    this->ui->Input->setText(this->bf_header.input);
}

void MainWindow::on_Input_textChanged(){
    this->bf_header.input = this->ui->Input->toPlainText();
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
    
    this->bf_header.max_memory = coeff*multiplier;
    
    printf("Setting max memory to %llu", this->bf_header.max_memory);
}

void MainWindow::on_maxInst_spinBox_valueChanged(int arg1){
    this->bf_header.max_instructions = arg1;
}

void MainWindow::on_start_pause_button_clicked(){
    if (this->bf_header.running){
        this->bf_header.stop = true;
        this->ui->start_pause_button->setText("Fuck it!!! (execute)"); //TODO: derive these strings from the same place
    } else {
        this->bf_header.reset_program();
        QTimer::singleShot(100, this, SLOT(runProgram()));
    }
}

void MainWindow::on_zoom_in_output_button_clicked(){
    this->ui->Output->zoomIn();
}

void MainWindow::on_zoom_out_output_button_clicked(){
    this->ui->Output->zoomOut();
}


void MainWindow::on_limit_inst_checkBox_stateChanged(int arg1){
    Qt::CheckState state = static_cast<Qt::CheckState>(arg1);
    switch(state){
    case Qt::Unchecked:
        this->bf_header.max_instructions_enforced = false;
        break;
    case Qt::Checked:
    case Qt::PartiallyChecked:
        this->bf_header.max_instructions_enforced = true;
        break;
    default:
        this->bf_header.max_instructions_enforced = false;
        break;
    }
    
    this->ui->max_inst_label->setEnabled(this->bf_header.max_instructions_enforced);
    this->ui->maxInst_spinBox->setEnabled(this->bf_header.max_instructions_enforced);
}

void MainWindow::on_max_mem_checkBox_stateChanged(int arg1){
    Qt::CheckState state = static_cast<Qt::CheckState>(arg1);
    switch(state){
    case Qt::Unchecked:
        this->bf_header.max_mem_enforced = false;
        break;
    case Qt::Checked:
    case Qt::PartiallyChecked:
        this->bf_header.max_mem_enforced = true;
        break;
    default:
        this->bf_header.max_mem_enforced = false;
        break;
    }
    
    this->ui->max_mem_label->setEnabled(this->bf_header.max_mem_enforced);
    this->ui->maxMem_spinBox->setEnabled(this->bf_header.max_mem_enforced);
    this->ui->max_mem_units_comboBox->setEnabled(this->bf_header.max_mem_enforced);
}
