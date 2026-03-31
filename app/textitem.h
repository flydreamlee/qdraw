#pragma once
// ---------------------------------------------------------------------------
// GraphicsTextItem — qdraw integration version
//
// Inherits from qdraw's GraphicsItem (which is AbstractShapeType<QGraphicsItem>)
// so it participates in the qdraw handle system, XML serialization, and scene
// management out of the box.
//
// Include this file INSTEAD of the standalone textitem.h when building inside
// the qdraw project.  The qdraw headers (drawobj.h) must be on the include path.
// ---------------------------------------------------------------------------
#include "drawobj.h"    // GraphicsItem, AbstractShape, SizeHandleRect (qdraw)

#include <QGraphicsTextItem>
#include <QFont>
#include <QImage>
#include <QKeyEvent>
#include <QVector>
#include <QString>
#include <QTimer>

// ---------------------------------------------------------------------------
// GraphicsTextItem
// ---------------------------------------------------------------------------
//class GraphicsTextItem : public GraphicsItem          // GraphicsItem = AbstractShapeType<QGraphicsItem>
class GraphicsTextItem : public GraphicsItem, QGraphicsTextItem
{
public:
    // qdraw requires a unique integer type id
    enum { Type = UserType + 100 };
    int type() const override { return Type; }

    //explicit GraphicsTextItem(QGraphicsItem *parent = nullptr);
    //GraphicsTextItem(const QRectF & rect , const QString &text, QGraphicsItem * parent= Q_NULLPTR);////explicit GraphicsTextItem(const QString &text, QGraphicsItem *parent = Q_NULLPTR);
    GraphicsTextItem(const QRectF & rect , QGraphicsItem * parent= Q_NULLPTR);//explicit GraphicsTextItem(QGraphicsItem *parent = Q_NULLPTR);
    void initialize(const QRectF &rect);

    ~GraphicsTextItem() override;

    // -----------------------------------------------------------------------
    // AbstractShape interface (qdraw pure-virtual overrides)
    // -----------------------------------------------------------------------
    void  paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option,
                QWidget *widget) override;

    //QRectF boundingRect() const override;

    QRectF boundingRect() const;
    void stretch(int handle , double sx , double sy , const QPointF & origin);
	void updateCoordinate();

    // Resize callback — qdraw calls this when a SizeHandleRect is dragged.
    // handle is the SizeHandleRect::Direction enum value.
//    void  resizeTo(SizeHandleRect::Direction dir, const QPointF &point) override;

protected:
    // Called when the item is moved (itemChange → ItemPositionHasChanged)
    void  updatehandles();// override;

public:
    // XML persistence
    bool  saveToXml(QXmlStreamWriter *xml) override;
    bool  loadFromXml(QXmlStreamReader *xml) override;

    // -----------------------------------------------------------------------
    // GraphicsTextItem-specific API (unchanged from standalone version)
    // -----------------------------------------------------------------------
    void    setText(const QString &text);
    QString text()    const { return m_text; }

    void   setTextWidth(qreal w);
    qreal  textWidth()  const { return m_width; }

    void   setItemHeight(qreal h);
    qreal  itemHeight() const { return m_height; }

    void          setAlignment(Qt::Alignment a);
    Qt::Alignment alignment()  const { return m_alignment; }

    void   setFont(const QFont &f);
    QFont  font() const { return m_font; }

    void   setScaleX(qreal sx);
    qreal  scaleX() const { return m_scaleX; }

    void   autoFitFont();

    void   setLocked(bool locked);
    bool   isLocked() const { return m_locked; }

    void   setTextRotation(int degrees);
    int    textRotation() const { return m_textRotation; }

    enum BorderSide { BorderTop = 1, BorderRight = 2, BorderBottom = 4, BorderLeft = 8 };
    void   setBorderSides(int sides);
    int    borderSides() const { return m_borderSides; }
    void   setBorderWidth(qreal w);
    qreal  borderWidth() const { return m_borderWidth; }

    void    setWordWrap(bool wrap);
    bool    wordWrap() const { return m_wordWrap; }
    QString getWrappedText() const;

    QImage  image() const;

    void setEditing(bool editing);
    bool isEditing() const { return m_editing; }

    void handleKeyPress(QKeyEvent *event);
    void handleMousePress(const QPointF &pos);

    bool isTextOverflowing() const;

//private:
    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------
    struct LineInfo {
        QString text;
        int     textStart;
        qreal   y;
    };
private:
    QVector<LineInfo> buildLines() const;
    void              recalcLocalRect();   // keeps m_localRect in sync

    // -----------------------------------------------------------------------
    // Data members
    // -----------------------------------------------------------------------
    QString       m_text;
    QFont         m_font;
//    qreal         m_width;
//    qreal         m_height;
    Qt::Alignment m_alignment;
    qreal         m_scaleX;
    bool          m_locked;
    int           m_textRotation;
    bool          m_wordWrap;
    int           m_borderSides;
    qreal         m_borderWidth;

    // Editing state
    bool   m_editing;
    int    m_cursorPos;
    bool   m_cursorVisible;
    QTimer m_cursorTimer;

    bool m_isRound;
    qreal m_fRatioY;
    qreal m_fRatioX;

    QRectF m_initialRect;
    QPointF opposite_;
    QPointF m_originPoint;

};
