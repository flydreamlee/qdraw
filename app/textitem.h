#pragma once
// ---------------------------------------------------------------------------
// GraphicsTextItem — qdraw-integrated text item
// Inherits GraphicsItem (AbstractShapeType<QGraphicsItem>) from drawobj.h
// ---------------------------------------------------------------------------
#include "drawobj.h"
#include <QFont>
#include <QImage>
#include <QKeyEvent>
#include <QVector>
#include <QTimer>

#define MIN_W		20.0
#define MIN_H		20.0

class GraphicsTextItem : public GraphicsItem
{
public:
    enum { Type = UserType + 100 };
    int type() const override { return Type; }

    //explicit GraphicsTextItem(QGraphicsItem *parent = nullptr);
    GraphicsTextItem(const QRectF & rect , QGraphicsItem * parent= Q_NULLPTR);
    void initialize(const QRectF &rect);

    ~GraphicsTextItem() override = default;

    // -----------------------------------------------------------------------
    // AbstractShape / GraphicsItem overrides
    // -----------------------------------------------------------------------
    QRectF boundingRect() const override;
    void   paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *option,
                 QWidget *widget) override;

    // Called by DrawScene during initial mouse-draw gesture (press→move→release)
    void stretch(int handle, double sx, double sy,
                 const QPointF &origin) override;

    // Called by DrawScene on mouseReleaseEvent to normalize geometry
    void updateCoordinate() override;

    // Called when a SizeHandleRect is dragged on an existing item
    //void resizeTo(SizeHandleRect::Direction dir, const QPointF &point) override;
    void resizeTo(int dir, const QPointF &point);

    // Called when item moves (from itemChange)
    void updatehandles() override;

    // XML persistence
    bool saveToXml(QXmlStreamWriter *xml) override;
    bool loadFromXml(QXmlStreamReader *xml) override;

    // Duplicate for copy/paste
    QGraphicsItem *duplicate() const override;

    // -----------------------------------------------------------------------
    // Text-specific API
    // -----------------------------------------------------------------------
    void    setText(const QString &text);
    QString text() const { return m_text; }

    void          setAlignment(Qt::Alignment a);
    Qt::Alignment alignment() const { return m_alignment; }

    void   setFont(const QFont &f);
    QFont  font() const { return m_font; }

    void   setScaleX(qreal sx);
    qreal  scaleX() const { return m_scaleX; }

    void   autoFitFont();

    void   setTextLocked(bool locked);
    bool   isTextLocked() const { return m_locked; }

    void   setTextRotation(int degrees);
    int    textRotation() const { return m_textRotation; }

    enum BorderSide { BorderTop=1, BorderRight=2, BorderBottom=4, BorderLeft=8 };
    void   setBorderSides(int sides);
    int    borderSides() const { return m_borderSides; }
    void   setBorderLineWidth(qreal w);
    qreal  borderLineWidth() const { return m_borderWidth; }

    void    setWordWrap(bool wrap);
    bool    wordWrap() const { return m_wordWrap; }
    QString getWrappedText() const;

    QImage  toImage() const;

    void setEditing(bool editing);
    bool isEditing() const { return m_editing; }

    bool isTextOverflowing() const;
/*
    // Minimum dimensions
    static constexpr qreal MIN_W = 60.0;
    static constexpr qreal MIN_H = 20.0;
*/

protected:
    // -----------------------------------------------------------------------
    // Qt event handlers — required for keyboard text input
    // -----------------------------------------------------------------------
    // Double-click enters edit mode
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    // Single click while editing places the cursor
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    // Forward all key presses to our text engine while editing
    void keyPressEvent(QKeyEvent *event) override;
    // Exit edit mode when the item loses focus
    void focusOutEvent(QFocusEvent *event) override;

private:
    // Internal text layout helper
    struct LineInfo {
        QString text;
        int     textStart;
        qreal   y;
    };
    QVector<LineInfo> buildLines() const;
    void handleMousePress(const QPointF &localPos);

    // Sync m_width/m_height → m_textWidth/m_textHeight and m_localRect
    void syncDimensions();

    // -----------------------------------------------------------------------
    // Data members
    // -----------------------------------------------------------------------
    QString       m_text;
    QFont         m_font;
    Qt::Alignment m_alignment;
    qreal         m_scaleX;
    bool          m_locked;
    int           m_textRotation;
    bool          m_wordWrap;
    int           m_borderSides;
    qreal         m_borderWidth;

    // Actual draw dimensions (kept >= MIN_W/MIN_H)
    qreal  m_textWidth;
    qreal  m_textHeight;

    QRectF m_initialRect;

    // Editing state
    bool   m_editing;
    int    m_cursorPos;
    bool   m_cursorVisible;
    QTimer m_cursorTimer;
};
