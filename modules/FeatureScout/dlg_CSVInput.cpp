#include "dlg_CSVInput.h"
#include "qfiledialog.h"
#include "qmessagebox.h"

dlg_CSVInput::dlg_CSVInput(QWidget * parent/* = 0,*/, Qt::WindowFlags f/* f = 0*/) : QDialog(parent, f) {

	this->setupUi(this);
	this->initParameters();
	this->buttonBox->setVisible(false);
	this->buttonBox->setDisabled(true);
	this->m_confParams = QSharedPointer<csvConfig::configPararams>(new csvConfig::configPararams());
	
	connectSignals();

}

dlg_CSVInput::~dlg_CSVInput()
{
	if (this->m_DataTableSelected) {
		delete this->m_DataTableSelected;
		this->m_DataTableSelected = nullptr;
	}


}

void dlg_CSVInput::connectSignals()
{
	connect(btn_loadCSV, SIGNAL(clicked()), this, SLOT(FileBtnClicked()));
	connect(btn_LoadConfig, SIGNAL(clicked()), this, SLOT(LoadFormatBtnClicked()));
	connect(btn_CustomFormat, SIGNAL(clicked()), this, SLOT(CustomFormatBtnClicked()));
	connect(btn_loadCols, SIGNAL(clicked()), this, SLOT(LoadColsBtnClicked()));
}

//enabling for custom file format
void dlg_CSVInput::CustomFormatBtnClicked(){
	bool enabled = true; 
	this->ed_startLine->setEnabled(true);
	this->ed_endLine->setEnabled(true);
	this->ed_decimal->setEnabled(true);
	this->ed_language->setEnabled(true);
	this->ed_Spacing->setEnabled(true);
}

void dlg_CSVInput::LoadColsBtnClicked()
{
	this->setSelectedEntries();
	this->buttonBox->setEnabled(true);
	this->buttonBox->setVisible(true);
}

void dlg_CSVInput::FileBtnClicked()
{
	this->myLayout->addWidget(this->m_entriesPreviewTable); 
	this->assignFileFormat(); 
	this->assignSeparator();
	this->AssignFormatLanguage(); 
	
	this->loadFilePreview(10); 
	this->showConfigParams(*this->m_confParams);
}

void dlg_CSVInput::AssignFormatLanguage() {
	if (cb_fmtEnglish->isChecked()) {
		this->m_confParams->csv_Inputlanguage = csvConfig::inputLang::EN;

	}
	else this->m_confParams->csv_Inputlanguage = csvConfig::inputLang::GER;
}

const csvConfig::configPararams& dlg_CSVInput::getConfigParameters() const {
	return *this->m_confParams; 
}


//shows configuration parameters to gui
void dlg_CSVInput::showConfigParams(const csvConfig::configPararams & params)
{
	this->ed_startLine->setText(QString("%1").arg(params.endLine));
	this->ed_endLine->setText(QString("%1").arg(params.startLine));
	this->cmb_box_separator->setCurrentIndex(0);
	this->ed_language->setText("EN");
	this->ed_Spacing->setText(QString("%1").arg(params.spacing));
	this->ed_Units->setText(QString("1").arg(params.csv_units)); 
}

void dlg_CSVInput::LoadFormatBtnClicked()
{
	
}

void dlg_CSVInput::initParameters(){
	
	if (!m_confParams)
		this->m_confParams = QSharedPointer<csvConfig::configPararams>(new csvConfig::configPararams());


	this->m_confParams->file_seperator = csvConfig::csvSeparator::Colunm;
	this->m_confParams->csv_Inputlanguage = csvConfig::inputLang::EN;
	this->m_confParams->file_fmt = csvConfig::csv_FileFormat::Default; 
	this->m_confParams->startLine = 1;
	this->m_confParams->endLine = 0; 
	this->m_confParams->spacing = 10.5f;
	this->m_confParams->csv_units = "microns";
	this->m_confParams->headerStartLine = 5; 
	this->m_confParams->paramsValid = false;
	this->m_fPath = "D:/OpenIa_TestDaten/Pores/";
	this->m_entriesPreviewTable = new dataTable(); 
	
	if (!this->m_currentHeaders) {
		this->m_currentHeaders = QSharedPointer<QStringList>(new QStringList()); 
	
	}
	if (!this->m_DataTableSelected) {
		this->m_DataTableSelected = new dataTable();
	
	}


	if (!this->m_selHeaders) {
		this->m_selHeaders = QSharedPointer < QStringList>(new QStringList());
	}

}

void dlg_CSVInput::resetDefault()
{
	this->initParameters(); 
}

bool dlg_CSVInput::validateParameters()
{

	return true; 
}

void dlg_CSVInput::setError(const QString &ParamName,const QString & Param_value )
{
	
	this->m_Error_Parameter.append("Error" + ParamName + "\t" + Param_value + "\n");
	this->m_confParams->paramsValid = false; 
}


void dlg_CSVInput::assignFileFormat() 
{
	//read out text of combo box
	QString CSVFileFormat = this->cmb_box_FileFormat->currentText();

	if (CSVFileFormat == "VolumeGraphics") {
		this->m_csvFileFormat = csvConfig::csv_FileFormat::VolumeGraphics;


	}
	else if (CSVFileFormat == "MAVI") {
		this->m_csvFileFormat = csvConfig::csv_FileFormat::MAVI;

	}
	else if (CSVFileFormat == "open_iA") {
		this->m_csvFileFormat = csvConfig::csv_FileFormat::open_IA_FeatureScout;
	}
	else {
		this->m_csvFileFormat = csvConfig::csv_FileFormat::Default;
	}
	
}

void dlg_CSVInput::assignSeparator() {
	bool param_seperator_ok = false;
	QString tmp_seperator = this->cmb_box_separator->currentText();
	
	if (tmp_seperator.contains(";"))
	{
		this->m_confParams->file_seperator = csvConfig::csvSeparator::Colunm;
		param_seperator_ok = true; 
	
	}else if (tmp_seperator.contains(",")) {
		this->m_confParams->file_seperator = csvConfig::csvSeparator::Comma;
		param_seperator_ok = true;
	}
	else {

		this->setError("Separator", tmp_seperator);
	}

	this->m_confParams->paramsValid = param_seperator_ok; 
	
	
}

void dlg_CSVInput::loadFilePreview(const int rowCount) {
	m_entriesPreviewTable->setColSeparator(this->m_confParams->file_seperator);
	
	if (!this->checkFile())
	{
		return; 
	}
	else
	{
		this->m_entriesPreviewTable->prepareTable(rowCount, this->m_confParams->colCount, this->m_confParams->headerStartLine); 
		this->loadEntries(this->m_confParams->fileName, rowCount);
		this->showPreviewTable(); 
	}
}


//checks if file exists and save it to config params
bool dlg_CSVInput::checkFile() {
	bool fileOK = false; 
	if (m_fPath.isEmpty()) {

		this->m_confParams->paramsValid = false;
		return fileOK;
	}

	QString fileName = QFileDialog::getOpenFileName(
		this, tr("Open Files"), m_fPath, tr("csv spreadsheet (*.csv),.csv")
	);

	if (fileName.isEmpty())
	{
		this->m_confParams->paramsValid = false;
		return fileOK;
	}
	else {
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly)) {
			QMessageBox::information(this, tr("Unable to open file"),
				file.errorString());
			this->setError(QString("unable to open file"), file.errorString());
			this->m_confParams->paramsValid = false;
			return fileOK;
		}
		else {
			this->m_confParams->paramsValid = (this->m_confParams->paramsValid && true);

			if (!this->m_confParams->paramsValid) {
				QMessageBox::information(this, tr("Wrong parameters assigned:"),
					this->m_Error_Parameter);
			}
			else {
				fileOK = true; 
				this->m_confParams->fileName = fileName;
				
				/*this->m_fPath = fileName;*/
				this->btn_loadCSV->setEnabled(false);
				this->btn_loadCSV->setVisible(false);
			}
		}

		if (file.isOpen()) {
			file.close(); 
		}
	}


	return fileOK; 

}

//loading entries in tablewidget preview
bool dlg_CSVInput::loadEntries(const QString& fileName, const unsigned int nrPreviewElements) {
	bool dataLoaded = false; 
	uint startElLine = (uint)  this->m_confParams->startLine;

	//TODO REmOVE hard coded columns in  dgl_csvInput
	dataLoaded = this->m_entriesPreviewTable->readTableEntries(fileName, nrPreviewElements, this->m_confParams->colCount,this->m_confParams->headerStartLine, &startElLine, true, false); 
	this->assignHeaderLine(); 
	return dataLoaded; 
}

void dlg_CSVInput::showPreviewTable()
{
	this->m_entriesPreviewTable->setAutoScroll(true);
	this->m_entriesPreviewTable->setEnabled(true);
	this->m_entriesPreviewTable->setVisible(true);
	this->m_entriesPreviewTable->update();
}


//assign headers and prepare map with indexes
void dlg_CSVInput::assignHeaderLine() {
	int autoIdxCol = 0; //
	if (!this->m_currentHeaders) return; 
	*this->m_currentHeaders = m_entriesPreviewTable->getHeaders(); 

	if (this->m_currentHeaders->isEmpty()) return;
	this->m_confParams->tableWidth = this->m_currentHeaders->length();
	
	for (const auto &currItem:*this->m_currentHeaders){
		if (!currItem.trimmed().isEmpty())
		{
				this->textControl_list->addItem(currItem);
				this->m_hashEntries.insert(currItem, autoIdxCol);
			
		}
			autoIdxCol++;
		
	}

	
	this->textControl_list->update(); 
}

//setEntries from a selected List;
void dlg_CSVInput::setSelectedEntries() {
	uint currItemIdx; 
	QString listEntry; 
	this->m_selectedHeadersList  = this->textControl_list->selectedItems();
				
	//no selection use all entries
	if (!(this->m_selectedHeadersList.length() == 0)) {
		for (const auto &selEntry : m_selectedHeadersList) {
			listEntry = selEntry->text();
			//selEntry->
			//selEntry->

			currItemIdx = this->m_hashEntries.value(listEntry);
			this->m_selColIdx.push_back(currItemIdx);
			this->m_selHeaders->append(listEntry);
			
		}
		
		qSort(this->m_selColIdx.begin(), this->m_selColIdx.end(), qLess<uint>());
		 
	}

}

const QVector<uint>& dlg_CSVInput::getEntriesSelInd()
{
	return this->m_selColIdx;
}


