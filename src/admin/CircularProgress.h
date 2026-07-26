#pragma once

#include <QWidget>

class CircularProgress : public QWidget {
    Q_OBJECT
public:
    explicit CircularProgress(QWidget *parent = nullptr);
    void setPercentage(int pct);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int pct_ = 0;
};
