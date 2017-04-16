#include "ExternalViewer.h"
#include "ExternalViewerTools.h"
#include "MainWindow.h"
#include "util/SymbolIconManager.h"
#include "sequence_view/SequenceView.h"

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
	viewersConfig->setMinimumWidth(100);
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

QIcon ExternalViewerTools::GetIconForViewer(const ExternalViewerConfig &config)
{
	QFileInfo info(config.iconPath);
	if (info.isRoot() || !info.exists()){
		info = QFileInfo(config.programPath);
	}
	if (info.isRoot() || !info.exists()){
		return QIcon(":/images/missing64.png");
	}else{
		return QFileIconProvider().icon(info);
	}
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
		viewersConfig->addItem(GetIconForViewer(c), c.displayName);
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
	delete dialog;
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
	//list->setMaximumWidth(160);
	list->setMinimumSize(80, 80);
	list->setMaximumWidth(200);
	list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	upButton = new QToolButton();
	downButton = new QToolButton();
	addButton = new QToolButton();
	removeButton = new QToolButton();
	upButton->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Up));
	downButton->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Down));
	addButton->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Plus));
	removeButton->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Minus));
	upButton->setToolTip(tr("Up"));
	downButton->setToolTip(tr("Down"));
	addButton->setToolTip(tr("Add"));
	removeButton->setToolTip(tr("Remove"));
	upButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	downButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	removeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	QSize iconSize(20, 20);
	upButton->setIconSize(iconSize);
	downButton->setIconSize(iconSize);
	addButton->setIconSize(iconSize);
	removeButton->setIconSize(iconSize);
	connect(upButton, SIGNAL(clicked(bool)), this, SLOT(Up()));
	connect(downButton, SIGNAL(clicked(bool)), this, SLOT(Down()));
	connect(addButton, SIGNAL(clicked(bool)), this, SLOT(Add()));
	connect(removeButton, SIGNAL(clicked(bool)), this, SLOT(Remove()));
	auto listEditButtonsLayout = new QHBoxLayout();
	listEditButtonsLayout->addStretch(1);
	listEditButtonsLayout->addWidget(upButton);
	listEditButtonsLayout->addWidget(downButton);
	listEditButtonsLayout->addWidget(removeButton);
	listEditButtonsLayout->addWidget(addButton);

	auto listLayout = new QVBoxLayout();
	listLayout->addWidget(list);
	listLayout->addLayout(listEditButtonsLayout);

	auto content = new QFormLayout();
	content->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	content->setSizeConstraint(QLayout::SetNoConstraint);
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
	argsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	argsLayout->setSizeConstraint(QLayout::SetNoConstraint);
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
	auto vars = new QGridLayout();
	vars->addWidget(VarLabel("$(filename)"), 0, 0); vars->addWidget(VarDescription(tr("file name")), 0, 1);
	vars->addWidget(VarLabel("$(directory)"), 1, 0); vars->addWidget(VarDescription(tr("document directory")), 1, 1);
	vars->addWidget(VarLabel("$(exedir)"), 2, 0); vars->addWidget(VarDescription(tr("application directory")), 2, 1);
	vars->addWidget(VarLabel("$(measure)"), 0, 2); vars->addWidget(VarDescription(tr("current measure")), 0, 3);
	vars->addWidget(VarLabel("$(time)"), 1, 2); vars->addWidget(VarDescription(tr("current time in seconds")), 1, 3);
	vars->addWidget(VarLabel("$(ticks)"), 2, 2); vars->addWidget(VarDescription(tr("current time in ticks")), 2, 3);
	vars->setColumnStretch(1, 1);
	vars->setColumnStretch(3, 1);
	vars->setColumnMinimumWidth(1, 20);
	vars->setColumnMinimumWidth(3, 20);
	auto varsWidget = new QGroupBox(tr("Defined variables"));
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
		auto item = NewListItem(c);
		list->addItem(item);
	}
	list->setCurrentRow(index);
	if (index < 0){
		// make sure
		PageChanged(-1);
	}
}

ExternalViewerConfigDialog::~ExternalViewerConfigDialog()
{
}

void ExternalViewerConfigDialog::Up()
{
	int i = list->currentRow();
	if (i < 0 || i == 0)
		return;
	auto c = config[i];
	config.removeAt(i);
	auto item = list->takeItem(i);
	i--;
	config.insert(i, c);
	list->insertItem(i, item);
	list->setCurrentRow(i);
}

void ExternalViewerConfigDialog::Down()
{
	int i = list->currentRow();
	if (i < 0 || i == config.size()-1)
		return;
	auto c = config[i];
	config.removeAt(i);
	auto item = list->takeItem(i);
	i++;
	config.insert(i, c);
	list->insertItem(i, item);
	list->setCurrentRow(i);
}

void ExternalViewerConfigDialog::Add()
{
	ExternalViewerConfig c = ExternalViewer::CreateNewConfig();
	int i = list->currentRow();
	if (i < 0)
		i = 0;
	config.insert(i, c);
	auto item = NewListItem(c);
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
	auto newIndex = i >= config.count() ? config.count() - 1 : i;
	list->setCurrentRow(newIndex);
	if (newIndex < 0){
		// make sure
		PageChanged(-1);
	}
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
		upButton->setEnabled(true);
		downButton->setEnabled(true);
		removeButton->setEnabled(true);
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
		upButton->setEnabled(false);
		downButton->setEnabled(false);
		removeButton->setEnabled(false);
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
	list->item(i)->setIcon(ExternalViewerTools::GetIconForViewer(config[i]));
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

QListWidgetItem *ExternalViewerConfigDialog::NewListItem(const ExternalViewerConfig &c)
{
	auto item = new QListWidgetItem(c.displayName);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item->setIcon(ExternalViewerTools::GetIconForViewer(c));
	return item;
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

QWidget *ExternalViewerConfigDialog::VarDescription(QString desc)
{
	auto label = new ElidableLabel(desc);
	label->SetElideMode(Qt::ElideRight);
	label->setMinimumWidth(20);
	label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
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



ElidableLabel::ElidableLabel(QString text, QWidget *parent)
	: QLabel(text, parent)
	, originalText(text)
	, elideMode(Qt::ElideNone)
{
}

void ElidableLabel::SetOriginalText(QString text)
{
	originalText = text;
	this->resize(size());
}

void ElidableLabel::SetElideMode(Qt::TextElideMode mode)
{
	elideMode = mode;
	this->resize(size());
}

void ElidableLabel::resizeEvent(QResizeEvent *event)
{
	QFontMetrics metrics(font());
	QString elidedText = metrics.elidedText(originalText, elideMode, width());
	setText(elidedText);
}
