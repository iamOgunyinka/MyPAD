#include "dialog.h"

Dialog::Dialog(QWidget *parent): QMainWindow(parent),
    textEdit(new QTextEdit(this)), newAction(nullptr), openAction(nullptr),
    saveAction(nullptr), quitAction(nullptr), copyAction(nullptr),
    cutAction(nullptr), replaceAction(nullptr),
    fileMenu(new QMenu(this)), editMenu(new QMenu(this)),
    toolBar(new QToolBar(this))
{
    this->setCentralWidget(textEdit);

    this->setWindowTitle(QString("%1 [*] - %2").arg("Untitled").arg(tr("JPad")));
    setWindowIcon(QIcon(tr(":/images/resources/logo.png")));

    addActions();
    enableActions();
    showMenubar();
    showToolbar();

    statusBar()->setToolTip(tr("Done"));

    QObject::connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
                     this, SLOT(setWindowModified(bool)));
}

void Dialog::addActions()
{
    newAction = new QAction(QIcon(tr(":/images/resources/new.png")), tr("&New File"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setStatusTip(tr("Create New File"));
    QObject::connect(newAction, SIGNAL(triggered()), this, SLOT(createNew()));

    openAction = new QAction(QIcon(tr(":/images/resources/open.png")), tr("&Open"), this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setStatusTip(tr("Open Text File"));
    QObject::connect(openAction, SIGNAL(triggered()), this, SLOT(openDoc()));

    saveAction = new QAction(QIcon(tr(":/images/resources/save.png")), tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save"));
    QObject::connect(saveAction, SIGNAL(triggered()), this, SLOT(saveText()));
    QObject::connect(textEdit->document(), SIGNAL(modificationChanged(bool)), saveAction, SLOT(setEnabled(bool)));

    quitAction = new QAction(QIcon(tr(":/images/resources/exit.png")), tr("Exit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    QObject::connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    copyAction = new QAction(QIcon(tr(":/images/resources/copy.png")), tr("&Copy"), this);
    copyAction->setShortcut(tr("Ctrl+C"));
    QObject::connect(textEdit, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)));
    QObject::connect(copyAction, SIGNAL(triggered()), textEdit, SLOT(copy()));

    cutAction = new QAction(QIcon(tr(":/images/resources/cut.png")), tr("Cut"), this);
    cutAction->setShortcut(tr("Ctrl+X"));
    QObject::connect(textEdit, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)));
    QObject::connect(cutAction, SIGNAL(triggered()), textEdit, SLOT(cut()));

    replaceAction = new QAction(QIcon(tr(":/images/resources/replace.png")), tr("Search & Replace"),
                                this);
    replaceAction->setShortcut((tr("Ctrl+R")));
    QObject::connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
                     replaceAction, SLOT(setEnabled(bool)));
    QObject::connect(replaceAction, SIGNAL(triggered()), this, SLOT(searchFunc()));
}

void Dialog::enableActions()
{
    bool selectionAvailable = textEdit->textCursor().hasSelection();
    cutAction->setEnabled(selectionAvailable);
    copyAction->setEnabled(selectionAvailable);
    replaceAction->setEnabled(selectionAvailable);
}
void Dialog::showMenubar()
{
    fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(quitAction);

    editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(copyAction);
    editMenu->addAction(cutAction);
    editMenu->addAction(replaceAction);
}

void Dialog::showToolbar()
{
    toolBar = addToolBar("&Menu");
    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addSeparator();
    toolBar->addAction(copyAction);
    toolBar->addAction(cutAction);
    toolBar->addAction(replaceAction);
    toolBar->addSeparator();
    toolBar->addAction(quitAction);
}

void Dialog::closeEvent(QCloseEvent *event)
{
    if(isSafeToClose("Close")) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool Dialog::isSafeToClose(const QString &title)
{
    if(isWindowModified()){
        switch(QMessageBox::information(this, title,
                          tr("Unsaved changes detected. Continue to %1 without saving?").arg(title.toLower()),
                          QMessageBox::Yes | QMessageBox::No))
        {
        case QMessageBox::No:
            return false;
        case QMessageBox::Yes: default:
            return true;
        }
    }
    return true;
}

void Dialog::createNew()
{
    (new Dialog(this))->show(); //smart pointers cannot be used here. 
}

void Dialog::setNewWindowTitle()
{
    if( m_filename.isEmpty() ){ return; }
    std::string new_filename { m_filename.toStdString() };
    int slashIndex = m_filename.lastIndexOf(tr("/")), dotIndex = m_filename.lastIndexOf(tr("."));
    if(slashIndex != m_filename.length() && dotIndex != m_filename.length()){
        new_filename = std::string(new_filename.begin() + slashIndex + 1,
                                   new_filename.begin() + dotIndex);
    }
    setWindowTitle(QString("%1 - %2").arg(new_filename.c_str()).arg(tr("JPad")));
}

void Dialog::saveText()
{
    if(m_filename.isEmpty()){
        m_filename = QFileDialog::getSaveFileName(this, tr("Save"), QDir::currentPath());
        if(m_filename.isNull()){
            return;
        }
    }
    setNewWindowTitle();
    if(this->isWindowModified()){
        saveFile();
    }
}

void Dialog::saveFile()
{
    QFile file(m_filename);
    if(file.open( QIODevice::WriteOnly )){
        QTextStream cout(&file);
        cout << textEdit->document()->toPlainText();
    }
    file.close();
}

void Dialog::openDoc()
{
    if( !isSafeToClose("Open") ) {
        return;
    }
    QString filename = QFileDialog::getOpenFileName(this, tr("Open"), QDir::currentPath(),
                                              QString("Text Documents (*.txt *.cpp *.cc)"));
    if(!filename.isNull() && (filename != m_filename)){
        m_filename = filename;
        QFile file(m_filename);
        if(file.open(QIODevice::ReadOnly)){
            QTextStream cin(&file);
            textEdit->document()->setPlainText(cin.readAll());
        }
        file.close();
    }
    setNewWindowTitle();
}

void Dialog::searchFunc()
{
    QString search = textEdit->textCursor().hasSelection() ?
                textEdit->textCursor().selectedText() : QString {};
    std::unique_ptr<ReplaceDialog> rDialog( new ReplaceDialog(search) );
    if(!rDialog){ return; }
    
    if( QDialog::Accepted == rDialog->exec() ){
        QString searchString = rDialog->getSearchString(), replaceString = rDialog->getReplaceString();
        bool replaceAll = rDialog->isReplaceAllChecked(), caseSensivity = rDialog->isSensitive();
        this->textEdit->moveCursor( QTextCursor::Start );

        do
        {
            if(textEdit->find(searchString, caseSensivity ?
                              QTextDocument::FindCaseSensitively: (QTextDocument::FindFlag)(0))){
                textEdit->insertPlainText(replaceString);
                QTextCursor c = textEdit->textCursor();
                c.select(QTextCursor::WordUnderCursor);
                textEdit->setTextCursor(c);
            } else {
                replaceAll = false;
            }
        } while (replaceAll);
    }
}

ReplaceDialog::ReplaceDialog(const QString &search, QDialog *parent): QDialog(parent),
    searchFor(nullptr), replaceWithString(nullptr),
    caseSensivity(nullptr), wholeDocument(nullptr),
    pushSearch(new QPushButton(tr("Ok"), this)),
    pushCancel(new QPushButton(tr("Cancel"), this)), gLayout(nullptr)
{
    searchFor = new QLineEdit(search.isEmpty() ? "" : search, this);
    gLayout = new QGridLayout(this);

    populateDialog();
    setSignals();

    setLayout(gLayout);
}

void ReplaceDialog::populateDialog()
{
    replaceWithString = new QLineEdit(this);
    caseSensivity = new QCheckBox("Case Sensitive", this);
    wholeDocument = new QCheckBox("Match Whole Document", this);

    gLayout->addWidget(new QLabel("Search for", this), 0, 0);
    gLayout->addWidget(searchFor, 0, 1);
    gLayout->addWidget(new QLabel("Replace with", this), 1, 0);
    gLayout->addWidget(replaceWithString, 1, 1);
    gLayout->addWidget(wholeDocument, 2, 0);
    gLayout->addWidget(caseSensivity, 3, 0);
    gLayout->addWidget(pushSearch, 4, 0);
    gLayout->addWidget(pushCancel, 4, 1);

    if(searchFor->text().isEmpty()){
        searchFor->setFocus();
    } else {
        replaceWithString->setFocus();
    }
}

inline QString ReplaceDialog::getReplaceString() const {
    return replaceWithString->text();
}
inline QString ReplaceDialog::getSearchString() const {
    return searchFor->text();
}
inline bool ReplaceDialog::isSensitive() const
{
    return caseSensivity->isChecked();
}
inline bool ReplaceDialog::isReplaceAllChecked() const
{
    return wholeDocument->isChecked();
}

void ReplaceDialog::setSignals()
{
    QObject::connect(pushSearch, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
}
