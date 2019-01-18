#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QDialog>

namespace Ui {
class DataViewer;
}

class DataViewer : public QDialog
{
    Q_OBJECT

public:
    explicit DataViewer(QWidget *parent, QString strWndTitle, float* fData, int iSizeX, int iSizeY);
    ~DataViewer();

private slots:
    void exportData(void);

private:
    Ui::DataViewer *ui;
};

#endif // DATAVIEWER_H
