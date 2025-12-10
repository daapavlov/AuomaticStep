#ifndef MOUSECLICKHANDLER_H
#define MOUSECLICKHANDLER_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QApplication>
#include <QTime>

class MouseClickHandler : public QObject {
    Q_OBJECT

public:
    explicit MouseClickHandler(QWidget *target, QObject *parent = nullptr)
        : QObject(parent), m_target(target), m_doubleClickInterval(QApplication::doubleClickInterval()) {

        if (m_target) {
            m_target->installEventFilter(this);
            m_target->setMouseTracking(true);
            m_target->setAttribute(Qt::WA_Hover); // Для событий hover
        }
    }

    // Установить таймаут для двойного клика (мс)
    void setDoubleClickTimeout(int ms) {
        m_doubleClickInterval = ms;
    }

    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj != m_target) {
            return QObject::eventFilter(obj, event);
        }

        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                handleMousePress(mouseEvent);
                return true; // Обработали событие
            }

            case QEvent::MouseButtonRelease: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                handleMouseRelease(mouseEvent);
                return true;
            }

            case QEvent::MouseButtonDblClick: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                handleDoubleClick(mouseEvent);
                return true;
            }

            case QEvent::MouseMove: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                handleMouseMove(mouseEvent);
                break;
            }

            case QEvent::Enter: {
                emit mouseEntered();
                break;
            }

            case QEvent::Leave: {
                emit mouseLeft();
                break;
            }

            case QEvent::HoverMove: {
                QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
                emit hoverMoved(hoverEvent->pos());
                break;
            }
        default:break;
        }

        return QObject::eventFilter(obj, event);
    }

signals:
    void clicked(Qt::MouseButton button, const QPoint &pos, Qt::KeyboardModifiers modifiers);
    void rightClicked(const QPoint &pos);
    void leftClicked(const QPoint &pos);
    void middleClicked(const QPoint &pos);
    void doubleClicked(Qt::MouseButton button, const QPoint &pos);
    void mousePressed(Qt::MouseButton button, const QPoint &pos);
    void mouseReleased(Qt::MouseButton button, const QPoint &pos);
    void mouseEntered();
    void mouseLeft();
    void hoverMoved(const QPoint &pos);
    void dragStarted(const QPoint &startPos);
    void dragMoved(const QPoint &delta);
    void dragEnded(const QPoint &endPos);

private:
    void handleMousePress(QMouseEvent *event) {
        m_pressPos = event->pos();
        m_pressTime = QTime::currentTime();
        m_pressButton = event->button();

        emit mousePressed(event->button(), event->pos());

        // Для правого клика можно сразу показать контекстное меню
        if (event->button() == Qt::RightButton) {
            emit rightClicked(event->pos());
        }
    }

    void handleMouseRelease(QMouseEvent *event) {
        emit mouseReleased(event->button(), event->pos());

        // Проверяем, был ли это клик (не drag)
        QPoint delta = event->pos() - m_pressPos;
        int timeDiff = m_pressTime.msecsTo(QTime::currentTime());

        if (delta.manhattanLength() < 10 && timeDiff < 500) {
            // Это был клик
            emit clicked(event->button(), event->pos(), event->modifiers());

            switch(event->button()) {
                case Qt::LeftButton: emit leftClicked(event->pos()); break;
                case Qt::RightButton: emit rightClicked(event->pos()); break;
                case Qt::MiddleButton: emit middleClicked(event->pos()); break;
                default: break;
            }
        }

        // Завершаем drag, если был
        if (m_isDragging) {
            m_isDragging = false;
            emit dragEnded(event->pos());
        }
    }

    void handleDoubleClick(QMouseEvent *event) {
        emit doubleClicked(event->button(), event->pos());
    }

    void handleMouseMove(QMouseEvent *event) {
        if (event->buttons() & m_pressButton) {
            // Кнопка зажата - это drag
            if (!m_isDragging) {
                m_isDragging = true;
                emit dragStarted(m_pressPos);
            }

            QPoint delta = event->pos() - m_lastDragPos;
            m_lastDragPos = event->pos();

            if (m_isDragging) {
                emit dragMoved(delta);
            }
        } else {
            m_lastDragPos = event->pos();
        }
    }

private:
    QWidget *m_target;
    bool m_isDragging = false;
    QPoint m_pressPos;
    QPoint m_lastDragPos;
    QTime m_pressTime;
    Qt::MouseButton m_pressButton;
    int m_doubleClickInterval;
};
#endif // MOUSECLICKHANDLER_H
