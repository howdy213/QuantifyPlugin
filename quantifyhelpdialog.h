#ifndef QUANTIFYHELPDIALOG_H
#define QUANTIFYHELPDIALOG_H

#include <QDialog>
#include <QString>
#include "WECore/WDef/wedef.h"
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class QuantifyHelpDialog; }
QT_END_NAMESPACE

class QuantifyHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuantifyHelpDialog(WConfigDocument *doc, QWidget *parent = nullptr);
    ~QuantifyHelpDialog();

private slots:
    void on_btnExample_clicked();

    void on_btnOpenDir_clicked();

private:
    // 初始化各个标签页
    void setupIntroTab();
    void setupQuantifyTab();
    void setupRecordTab();
    void setupRuleTab();
    void setupGroupTab();
    void setupConfigTab();
    void setupNamelistTab();

    // 辅助函数（用于创建示例文件）
    bool createTemplateFile(const QString &filePath, const QString &content);
    bool createNamelistExcel(const QString &filePath);
    bool createConfigFile(const QString &configPath,
                          const QDir &baseDir,
                          const QString &termDirName,
                          const QString &engineType);

    Ui::QuantifyHelpDialog *ui;
    WConfigDocument *m_doc;
};

#endif // QUANTIFYHELPDIALOG_H
