#ifndef CIRCULARPROGRESS_H
#define CIRCULARPROGRESS_H

#include <QWidget>

class CircularProgress : public QWidget {
    Q_OBJECT
public:
    explicit CircularProgress(QWidget *parent = nullptr);
    void setPercentage(int pct);
    int percentage() const { return pct_; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int pct_ = 0;
};

#endif
