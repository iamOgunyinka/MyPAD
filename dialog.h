#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
#include <QtWidgets>
#include <memory>

class Dialog : public QMainWindow
{
    Q_OBJECT
    
    QTextEdit *textEdit;
    QAction *newAction, *openAction, *saveAction, *quitAction,
            *copyAction, *cutAction, *replaceAction;
    QMenu *fileMenu, *editMenu;
    QToolBar *toolBar;
    bool isSafeToClose(const QString &title);
    QString m_filename;
    void saveFile();
    void setNewWindowTitle();
public:
    explicit Dialog(QWidget *parent = 0);
    void addActions();
    void enableActions();
    void showMenubar();
    void showToolbar();
    void closeEvent(QCloseEvent *event);
signals:
    //~ void documentModified(bool isDocModified);
public slots:
    void createNew();
    void saveText();
    void openDoc();
    void searchFunc();
};

struct ReplaceDialog : public QDialog
{
    explicit ReplaceDialog(const QString &search = QString{}, QDialog *parent = nullptr);
    QString getSearchString() const ;
    QString getReplaceString() const;
    void populateDialog();
    void setSignals();

    bool isReplaceAllChecked() const;
    bool isSensitive() const;
private:
    QLineEdit *searchFor, *replaceWithString;
    QCheckBox *caseSensivity, *wholeDocument;
    QPushButton *pushSearch, *pushCancel;
    QGridLayout *gLayout;
};
#endif // DIALOG_H
