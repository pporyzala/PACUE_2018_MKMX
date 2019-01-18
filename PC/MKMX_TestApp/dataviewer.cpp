#include "dataviewer.h"
#include "ui_dataviewer.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include <QDebug>

DataViewer::DataViewer(QWidget *parent, QString strWndTitle, float *fData, int iSizeX, int iSizeY) :
    QDialog(parent),
    ui(new Ui::DataViewer)
{
    ui->setupUi(this);

    setWindowTitle(strWndTitle);

    connect(ui->exportBtn, SIGNAL(clicked(bool)), this, SLOT(exportData()));
    connect(ui->okBtn, SIGNAL(clicked(bool)), this, SLOT(close()));

    ui->dataTable->setSortingEnabled(false);

    ui->dataTable->setColumnCount(iSizeX);
    ui->dataTable->setRowCount(iSizeY);

    qDebug() << "iSizeX: " << iSizeX << "iSizeY: " << iSizeY;

    for (int i = 0; i < iSizeY; i++) {
        QTableWidgetItem *newItem = new QTableWidgetItem(QString::number(fData[i], 'f', 6));
        ui->dataTable->setItem(i-1, 1, newItem);
    }
}

void DataViewer::exportData(void) {
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Zapisz dane..."), "",
            tr("Text file (*.txt);"));

    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Nie można otworzyć pliku ?!"), file.errorString());
            return;
        }

        QTextStream out(&file);
        for (int j = 0; j < ui->dataTable->rowCount(); j++) {
            for (int i = 0; i < ui->dataTable->columnCount(); i++) {
                out << ui->dataTable->item(j ,i)->text() << ", ";
            }
            out << "\n";
        }

        file.close();
    }
}

DataViewer::~DataViewer()
{
    delete ui;
}
