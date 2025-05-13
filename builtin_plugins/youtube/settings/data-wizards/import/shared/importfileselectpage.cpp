#include "importfileselectpage.h"
#include <QBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>

ImportFileSelectPage::ImportFileSelectPage(const QString& title, const QString& subtitle, const QString& targetFile,
                                           int nextPage, QWidget* parent)
    : QWizardPage(parent),
      browseButton(new QPushButton("...", this)),
      browseLayout(new QHBoxLayout),
      layout(new QVBoxLayout(this)),
      nextPage(nextPage),
      pathEdit(new QLineEdit(this)),
      targetFile(targetFile)
{
    setCommitPage(true);
    setSubTitle(subtitle);
    setTitle(title);

    pathEdit->setReadOnly(true);

    browseLayout->addWidget(pathEdit);
    browseLayout->addWidget(browseButton);
    layout->addLayout(browseLayout);

    connect(browseButton, &QPushButton::clicked, this, &ImportFileSelectPage::browseForFile);
}

void ImportFileSelectPage::browseForFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(), targetFile);
    if (fileName.isNull())
    {
        pathEdit->setText("No file selected");
        return;
    }

    emit fileSelected(fileName);
}

bool ImportFileSelectPage::isComplete() const
{
    return QFile::exists(pathEdit->text());
}
