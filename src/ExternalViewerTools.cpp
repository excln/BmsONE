#include "ExternalViewer.h"
#include "ExternalViewerTools.h"
#include "MainWindow.h"
#include "SymbolIconManager.h"
#include "SequenceView.h"

ExternalViewerTools::ExternalViewerTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow)
	: QToolBar(windowTitle, mainWindow)
	, mainWindow(mainWindow)
	, viewer(mainWindow->GetExternalViewer())
	, freezeIndexChange(false)
{
	setObjectName(objectName);
	setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

	actionPlayBeg = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::PlayZero), tr("Play from Beginning"));
	actionPlayHere = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Play), tr("Play from Here"));
	actionStop = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Stop), tr("Stop"));

	actionPlayBeg->setShortcuts(QList<QKeySequence>() << Qt::Key_F5 << Qt::ControlModifier + Qt::Key_R);
	actionPlayHere->setShortcuts(QList<QKeySequence>() << Qt::Key_F6 << Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_R);

	addWidget(viewersConfig = new QComboBox(this));
	actionConfigure = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Settings), tr("Configure External Viewers..."));

	mainWindow->GetMenuPreview()->addAction(actionPlayBeg);
	mainWindow->GetMenuPreview()->addAction(actionPlayHere);
	mainWindow->GetMenuPreview()->addAction(actionStop);
	mainWindow->GetMenuPreview()->addSeparator();
	menuViewers = mainWindow->GetMenuPreview()->addMenu(tr("Viewer"));
	mainWindow->GetMenuPreview()->addAction(actionConfigure);
	viewersActionGroup = new QActionGroup(this);

	connect(actionPlayBeg, SIGNAL(triggered(bool)), this, SLOT(PlayBeg()));
	connect(actionPlayHere, SIGNAL(triggered(bool)), this, SLOT(PlayHere()));
	connect(actionStop, SIGNAL(triggered(bool)), this, SLOT(Stop()));

	connect(viewersConfig, SIGNAL(currentIndexChanged(int)), this, SLOT(CurrentConfigIndexChanged(int)));
	connect(actionConfigure, SIGNAL(triggered(bool)), this, SLOT(Configure()));

	connect(viewer, SIGNAL(ConfigChanged()), this, SLOT(ConfigChanged()));
	connect(viewer, SIGNAL(ConfigIndexChanged(int)), this, SLOT(ConfigIndexChanged(int)));
	ConfigChanged();
}

ExternalViewerTools::~ExternalViewerTools()
{
}

void ExternalViewerTools::ConfigChanged()
{
	SetPlayable(viewer->IsPlayable());
	freezeIndexChange = true;
	viewersConfig->clear();
	menuViewers->clear();
	for (auto a : viewersActionGroup->actions()){
		viewersActionGroup->removeAction(a);
	}
	auto config = viewer->GetConfig();
	for (int i=0; i<config.size(); i++){
		auto c = config[i];
		viewersConfig->addItem(c.displayName);
		auto action = new QAction(c.displayName);
		action->setCheckable(true);
		connect(action, &QAction::triggered, [=](){
			ConfigIndexChanged(i);
		});
		menuViewers->addAction(action);
		viewersActionGroup->addAction(action);
	}
	freezeIndexChange = false;
	if (viewer->IsPlayable()){
		viewersConfig->setCurrentIndex(viewer->GetCurrentConfigIndex());
		viewersActionGroup->actions()[viewer->GetCurrentConfigIndex()]->setChecked(true);
	}
}

void ExternalViewerTools::ConfigIndexChanged(int i)
{
	if (viewersConfig->currentIndex() != i){
		viewersConfig->setCurrentIndex(i);
	}
	auto actions = menuViewers->actions();
	for (int j=0; j<actions.size(); j++){
		actions[j]->setChecked(i == j);
	}
	SetPlayable(viewer->IsPlayable());
}

void ExternalViewerTools::PlayBeg()
{
	viewer->PlayBeg();
}

void ExternalViewerTools::PlayHere()
{
	int time = mainWindow->GetActiveSequenceView()->GetCurrentLocation();
	viewer->Play(time);
}

void ExternalViewerTools::Stop()
{
	viewer->Stop();
}

void ExternalViewerTools::CurrentConfigIndexChanged(int i)
{
	if (!freezeIndexChange && i >= 0 && i < viewer->GetConfig().size()){
		viewer->SetConfigIndex(i);
		SetPlayable(viewer->IsPlayable());
	}
}

void ExternalViewerTools::Configure()
{
	auto dialog = new ExternalViewerConfigDialog(mainWindow, viewer->GetConfig(), viewer->GetCurrentConfigIndex());
	if (dialog->exec() == QDialog::Accepted){
		viewer->SetConfig(dialog->GetConfig(), dialog->GetIndex());
	}
}

void ExternalViewerTools::SetPlayable(bool playable)
{
	actionPlayBeg->setEnabled(playable);
	actionPlayHere->setEnabled(playable);
	actionStop->setEnabled(playable);
}




ExternalViewerConfigDialog::ExternalViewerConfigDialog(MainWindow *mainWindow, QList<ExternalViewerConfig> config, int i)
	: QDialog(mainWindow)
	, config(config)
	, index(i >= 0 && i < config.size() ? i : -1)
{
	setModal(true);
	UIUtil::SetFont(this);
	setWindowTitle(tr("External Viewer Configuration"));
	auto buttonsLayout = new QHBoxLayout();
	auto okButton = new QPushButton(tr("OK"));
	auto cancelButton = new QPushButton(tr("Cancel"));
	okButton->setDefault(true);
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	list = new QListWidget();
	list->setIconSize(QSize(32, 32));
	list->setMovement(QListView::Static);
	list->setMaximumWidth(160);

	auto addButton = new QPushButton();
	auto removeButton = new QPushButton();
	addButton->setText("+");
	removeButton->setText("-");
	addButton->setMaximumWidth(45);
	removeButton->setMaximumWidth(45);
	connect(addButton, SIGNAL(clicked(bool)), this, SLOT(Add()));
	connect(removeButton, SIGNAL(clicked(bool)), this, SLOT(Remove()));
	auto listEditButtonsLayout = new QHBoxLayout();
	listEditButtonsLayout->addStretch(1);
	listEditButtonsLayout->addWidget(addButton);
	listEditButtonsLayout->addWidget(removeButton);

	auto listLayout = new QVBoxLayout();
	listLayout->addWidget(list);
	listLayout->addLayout(listEditButtonsLayout);

	auto content = new QFormLayout();
	content->addRow(tr("Name:"), displayName = new QLineEdit());
	programPath = new QLineEdit();
	auto programField = new QHBoxLayout();
	selectProgramButton = new QToolButton();
	selectProgramButton->setText("...");
	connect(selectProgramButton, SIGNAL(clicked(bool)), this, SLOT(SelectProgram()));
	programField->addWidget(programPath);
	programField->addWidget(selectProgramButton);
	content->addRow(tr("Program Path:"), programField);
	content->addRow(tr("Working Directory:"), execDirectory = new QLineEdit());
	auto argsWidget = new QGroupBox(tr("Arguments"));
	auto argsLayout = new QFormLayout();
	argsLayout->addRow(LabelWithIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::PlayZero), tr("Play from Beginning:")), argPlayBeg = new QLineEdit());
	argsLayout->addRow(LabelWithIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Play), tr("Play from Here:")), argPlayHere = new QLineEdit());
	argsLayout->addRow(LabelWithIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Stop), tr("Stop:")), argStop = new QLineEdit());
	argsWidget->setLayout(argsLayout);
	content->addRow(argsWidget);
	connect(displayName, SIGNAL(editingFinished()), this, SLOT(DisplayNameEdited()));
	connect(programPath, SIGNAL(editingFinished()), this, SLOT(ProgramPathEdited()));
	connect(argPlayBeg, SIGNAL(editingFinished()), this, SLOT(ArgPlayBegEdited()));
	connect(argPlayHere, SIGNAL(editingFinished()), this, SLOT(ArgPlayHereEdited()));
	connect(argStop, SIGNAL(editingFinished()), this, SLOT(ArgStopEdited()));
	connect(execDirectory, SIGNAL(editingFinished()), this, SLOT(ExecDirectoryEdited()));
	variableAcceptingWidgets.append(argPlayBeg);
	variableAcceptingWidgets.append(argPlayHere);
	variableAcceptingWidgets.append(argStop);
	variableAcceptingWidgets.append(execDirectory);
	content->addRow(new QLabel(tr("Defined variables:")));
	auto vars = new QGridLayout();
	vars->addWidget(VarLabel("$(filename)"), 0, 0); vars->addWidget(new QLabel(tr("file name")), 0, 1);
	vars->addWidget(VarLabel("$(directory)"), 1, 0); vars->addWidget(new QLabel(tr("document directory")), 1, 1);
	vars->addWidget(VarLabel("$(exedir)"), 2, 0); vars->addWidget(new QLabel(tr("application directory")), 2, 1);
	vars->addWidget(VarLabel("$(measure)"), 0, 2); vars->addWidget(new QLabel(tr("current measure")), 0, 3);
	vars->addWidget(VarLabel("$(time)"), 1, 2); vars->addWidget(new QLabel(tr("current time in seconds")), 1, 3);
	vars->addWidget(VarLabel("$(ticks)"), 2, 2); vars->addWidget(new QLabel(tr("current time in ticks")), 2, 3);
	vars->setColumnStretch(1, 1);
	vars->setColumnStretch(3, 1);
	vars->setColumnMinimumWidth(1, 20);
	vars->setColumnMinimumWidth(3, 20);
	auto varsWidget = new QWidget();
	varsWidget->setLayout(vars);
	content->addRow(varsWidget);

	auto bodyLayout = new QHBoxLayout();
	bodyLayout->addLayout(listLayout);
	bodyLayout->addLayout(content, 1);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addLayout(bodyLayout, 1);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	connect(list, SIGNAL(currentRowChanged(int)), this, SLOT(PageChanged(int)));
	connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

	for (auto c : config){
		auto item = new QListWidgetItem(c.displayName, list);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
	list->setCurrentRow(index);
	if (index < 0){
		PageChanged(-1);
	}
}

ExternalViewerConfigDialog::~ExternalViewerConfigDialog()
{
}

void ExternalViewerConfigDialog::Add()
{
	ExternalViewerConfig c = ExternalViewer::CreateNewConfig();
	int i = list->currentRow();
	if (i < 0)
		i = 0;
	config.insert(i, c);
	auto item = new QListWidgetItem(c.displayName);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	list->insertItem(i, item);
	list->setCurrentRow(i);
}

void ExternalViewerConfigDialog::Remove()
{
	int i = list->currentRow();
	if (i < 0)
		return;
	list->takeItem(i);
	config.removeAt(i);
	list->setCurrentRow(i >= config.count() ? config.count() - 1 : i);
}

void ExternalViewerConfigDialog::PageChanged(int i)
{
	if (i >= 0 && i < config.size()){
		auto c = config[i];
		displayName->setText(c.displayName);
		programPath->setText(c.programPath);
		argPlayBeg->setText(c.argumentFormatPlayBeg);
		argPlayHere->setText(c.argumentFormatPlayHere);
		argStop->setText(c.argumentFormatStop);
		execDirectory->setText(c.executionDirectory);
		selectProgramButton->setEnabled(true);
		displayName->setEnabled(true);
		programPath->setEnabled(true);
		argPlayBeg->setEnabled(true);
		argPlayHere->setEnabled(true);
		argStop->setEnabled(true);
		execDirectory->setEnabled(true);
		this->index = i;
	}else{
		displayName->setText("");
		programPath->setText("");
		argPlayBeg->setText("");
		argPlayHere->setText("");
		argStop->setText("");
		execDirectory->setText("");
		selectProgramButton->setEnabled(false);
		displayName->setEnabled(false);
		programPath->setEnabled(false);
		argPlayBeg->setEnabled(false);
		argPlayHere->setEnabled(false);
		argStop->setEnabled(false);
		execDirectory->setEnabled(false);
	}
}

void ExternalViewerConfigDialog::DisplayNameEdited()
{
	int i = list->currentRow();
	if (i < 0)
		return;
	config[i].displayName = displayName->text();
	list->item(i)->setText(displayName->text());
}

void ExternalViewerConfigDialog::ProgramPathEdited()
{
	int i = list->currentRow();
	if (i < 0)
		return;
	config[i].programPath = programPath->text();
}

void ExternalViewerConfigDialog::ArgPlayBegEdited()
{
	int i = list->currentRow();
	if (i < 0)
		return;
	config[i].argumentFormatPlayBeg = argPlayBeg->text();
}

void ExternalViewerConfigDialog::ArgPlayHereEdited()
{
	int i = list->currentRow();
	if (i < 0)
		return;
	config[i].argumentFormatPlayHere = argPlayHere->text();
}

void ExternalViewerConfigDialog::ArgStopEdited()
{
	int i = list->currentRow();
	if (i < 0)
		return;
	config[i].argumentFormatStop = argStop->text();
}

void ExternalViewerConfigDialog::ExecDirectoryEdited()
{
	int i = list->currentRow();
	if (i < 0)
		return;
	config[i].executionDirectory = execDirectory->text();
}

void ExternalViewerConfigDialog::SelectProgram()
{
#ifdef Q_OS_WIN
	QString filter = tr("Executables (*.exe)\nAll files (*.*)");
#else
	QString filter = tr("All files (*.*)");
#endif
	QString path = QFileDialog::getOpenFileName(this, tr("Select program"), programPath->text(), filter);
	if (!path.isNull()){
		int i = list->currentRow();
		if (i < 0)
			return;
		config[i].programPath = QDir::toNativeSeparators(path);
		programPath->setText(config[i].programPath);
	}
}

QWidget *ExternalViewerConfigDialog::VarLabel(QString var)
{
	auto *label = new QPushButton();
	label->setText(var);
	label->setFocusPolicy(Qt::NoFocus);
	connect(label, &QPushButton::clicked, [=](bool){
		QWidget *focus = qApp->focusWidget();
		for (auto widget : variableAcceptingWidgets){
			if (static_cast<QWidget*>(widget) == focus){
				widget->insert(var);
				// make sure
				widget->editingFinished();
				break;
			}
		}
	});
	return label;
}

QWidget *ExternalViewerConfigDialog::LabelWithIcon(QIcon icon, QString text)
{
	auto widget = new QWidget();
	auto layout = new QHBoxLayout();
	auto iconLabel = new QLabel();
	iconLabel->setPixmap(icon.pixmap(16, 16));
	layout->setMargin(0);
	//layout->setSpacing(0);
	layout->addWidget(iconLabel);
	layout->addWidget(new QLabel(text), 1);
	widget->setLayout(layout);
	return widget;
}
