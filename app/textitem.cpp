// ---------------------------------------------------------------------------
// GraphicsTextItem — qdraw-integrated implementation
//
// Depends on:  drawobj.h / drawobj.cpp  (qdraw project, app/)
// ---------------------------------------------------------------------------
#include "textitem.h"
#include <QPainter>
#include <QFontMetricsF>
#include <QTextLayout>
#include <QTextOption>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QImage>
#include <QtMath>


// qdraw's SizeHandleRect::Direction enum values (from drawobj.h):
//   LeftTop=0, Top=1, RightTop=2, Right=3,
//   RightBottom=4, Bottom=5, LeftBottom=6, Left=7

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------
/*
GraphicsTextItem::GraphicsTextItem(QGraphicsItem *parent)
    : GraphicsItem(parent)          // calls qdraw's GraphicsItem ctor (creates 8 handles)
    , m_text("")
    , m_width(200.0)
    , m_height(40.0)
    , m_alignment(Qt::AlignLeft | Qt::AlignTop)
    , m_scaleX(1.0)
    , m_locked(false)
    , m_textRotation(0)
    , m_wordWrap(false)
    , m_borderSides(0)
    , m_borderWidth(2.0)
    , m_editing(false)
    , m_cursorPos(0)
    , m_cursorVisible(false)
{
    m_font.setFamily("Arial");
    m_font.setPointSize(14);

    QFontMetricsF fm(m_font);
    m_height = qMax(20.0, fm.height() + 8.0);

    // Set the qdraw local rect (used by GraphicsItem for handle positioning)
    recalcLocalRect();

    // Cursor blink
    QObject::connect(&m_cursorTimer, &QTimer::timeout, [this]() {
        m_cursorVisible = !m_cursorVisible;
        update();
    });
}
*/
GraphicsTextItem::GraphicsTextItem(const QRectF & rect ,QGraphicsItem * parent)
     : GraphicsItem(parent)//,
      //dd(new GraphicsTextItemPrivate)
{
    initialize(rect);
}

void GraphicsTextItem::initialize(const QRectF &rect)
{
    m_text = "";
    m_width = 200.0;
    m_height = 40.0;
    m_alignment = Qt::AlignLeft | Qt::AlignTop;
    m_scaleX = 1.0;
    m_locked = false;
    m_textRotation = 0;
    m_wordWrap = false;
    m_borderSides = 0;
    m_borderWidth = 2.0;
    m_editing = false;
    m_cursorPos = 0;
    m_cursorVisible = false;

    m_font.setFamily("Arial");
    m_font.setPointSize(14);

    QFontMetricsF fm(m_font);
//    m_height = qMax(20.0, fm.height() + 8.0);

m_width = rect.width();
m_height = rect.height();
m_initialRect = rect;
m_localRect = rect;
m_originPoint = QPointF(0,0);

    // Set the qdraw local rect (used by GraphicsItem for handle positioning)
    //recalcLocalRect();

    // Cursor blink
    QObject::connect(&m_cursorTimer, &QTimer::timeout, [this]() {
        m_cursorVisible = !m_cursorVisible;
        update();
    });
/*
    m_bTextFirstSel = true;
    setAcceptDrops(true); // 不可放在initial()
    setAcceptHoverEvents(true);// 不可放在initial()
    setFlag(ItemUsesExtendedStyleOption);
    setTextInteractionFlags(Qt::TextEditorInteraction);
    initialGraphicItem();
    m_font = ReadFontSetting();
    m_bwordWrap = false;//ReadItemDefaultSetting("Property","WordWrap",DefaultTextWordWrap).toBool();
    m_bautoPointSize = false;//ReadItemDefaultSetting("Property","AutoPoint",DefaultTextAutoPoint).toBool();
    m_bautoStretch = false;//ReadItemDefaultSetting("Property","AutoStretch",DefaultTextAutoStretch).toBool();
    m_Tbordertype = (TBorderType)(ReadItemDefaultSetting("Property","Border",DefaultTextBorder).toInt());
    m_ntextAlign = (Qt::Alignment)(ReadItemDefaultSetting("Property","Align",DefaultTextAlign).toInt());
    m_TextColor = QColor(Qt::black);//ReadItemDefaultSetting("Property","Color",DefaultItemColor).value<QColor>();
    m_textAngle = 0;
    m_bIsDate = false;
    if(m_bIsDate)       m_textSource = TextSource::PrintDate;
    else                m_textSource = TextSource::Keyboard;

    m_font.setKerning(m_bautoPointSize);
    m_font.setOverline(m_bautoStretch); //show auto size
    m_font.setStretch(100);//setStretch(100);


    if(dd->textControl())
    {
        QTextOption textoption = dd->control->document()->defaultTextOption();
        if(m_bwordWrap)
            textoption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        else
            textoption.setWrapMode(QTextOption::NoWrap);
        textoption.setAlignment(m_ntextAlign);
        dd->control->document()->setDefaultTextOption(textoption);
        dd->control->document()->setDefaultFont(m_font);
    }
    setDefaultTextColor(m_TextColor);
    m_lineSpace = 0;
    setLineSpace(100);

    m_width = rect.width();
    m_height = rect.height();
    updateWidthHeightToValue();
    m_initialRect = QRectF(-m_width/2,-m_height/2,m_width,m_height);//rect;
    m_localRect = m_initialRect;
    setTextWidth(textboundingRect().width());//setTextWidth(m_width);
    //dd->control->document()->setPageSize(m_localRect.size());
*/
    updatehandles();
}

GraphicsTextItem::~GraphicsTextItem() {}

// ---------------------------------------------------------------------------
// Keep qdraw's m_localRect in sync with our dimensions.
// GraphicsItem stores its logical rect in m_localRect (QRectF), which it
// uses to position the 8 SizeHandleRect children.
// ---------------------------------------------------------------------------
void GraphicsTextItem::recalcLocalRect()
{
    // m_localRect is the protected QRectF in AbstractShapeType<QGraphicsItem>
    m_localRect = QRectF(0, 0, m_width, m_height);
}

// ---------------------------------------------------------------------------
// boundingRect — include a small margin so handles are clickable
// ---------------------------------------------------------------------------
QRectF GraphicsTextItem::boundingRect() const
{
    return m_localRect;
/*
    const qreal extra = 5.0;
    return QRectF(-extra, -extra,
                  m_width  + extra * 2,
                  m_height + extra * 2);
*/
}

//copy from GraphicsRectItem::stretch()
void GraphicsTextItem::stretch(int handle , double sx, double sy, const QPointF & origin)
{
    QTransform trans  ;
    switch (handle) {
    case Right:
    case Left:
        sy = 1;
        break;
    case Top:
    case Bottom:
        sx = 1;
        break;
    default:
        break;
    }

    opposite_ = origin;

    trans.translate(origin.x(),origin.y());
    trans.scale(sx,sy);
    trans.translate(-origin.x(),-origin.y());

    prepareGeometryChange();
    m_localRect = trans.mapRect(m_initialRect);
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updatehandles();

}

void GraphicsTextItem::updateCoordinate()
{
    QPointF pt1,pt2,delta;

    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(m_localRect.center());
    delta = pt1 - pt2;

    //if (!parentItem() ){
        prepareGeometryChange();
        m_localRect = QRectF(-m_width/2,-m_height/2,m_width,m_height);//QRectF(0,0,m_width,m_height);//QRectF(-m_width/2,-m_height/2,m_width,m_height);//
        m_width = m_localRect.width();
        m_height = m_localRect.height();
//        updateWidthHeightToValue();
        setTransform(transform().translate(delta.x(),delta.y()));
        setTransformOriginPoint(m_localRect.center());
        moveBy(-delta.x(),-delta.y());
        setTransform(transform().translate(-delta.x(),-delta.y()));
        opposite_ = QPointF(0,0);
        updatehandles();
    //}
    m_initialRect = m_localRect;
}

// ---------------------------------------------------------------------------
// updateHandles — reposition the 8 qdraw SizeHandleRect children.
// Called by GraphicsItem::itemChange whenever the item moves or resizes.
// ---------------------------------------------------------------------------
void GraphicsTextItem::updatehandles()
{
    // Reuse qdraw's built-in positioning by syncing m_localRect first
    //recalcLocalRect();

    // qdraw's GraphicsItem::updateHandles() positions handles from m_localRect
    GraphicsItem::updatehandles();
}
/*
// ---------------------------------------------------------------------------
// resizeTo — qdraw calls this when the user drags a SizeHandleRect.
// dir is the SizeHandleRect::Direction value; point is the new scene pos.
// We translate it into changes to m_width / m_height / pos().
// ---------------------------------------------------------------------------
void GraphicsTextItem::resizeTo(SizeHandleRect::Direction dir, const QPointF &point)
{
    // point is in scene coordinates; convert to item-local coordinates
    QPointF localPt = mapFromScene(point);

    prepareGeometryChange();

    switch (dir) {
    case SizeHandleRect::LeftTop:
    {
        qreal newW = qMax(60.0, m_localRect.right()  - localPt.x());
        qreal newH = qMax(20.0, m_localRect.bottom() - localPt.y());
        QPointF delta(m_width - newW, m_height - newH);
        setPos(pos() + delta);
        m_width  = newW;
        m_height = newH;
        break;
    }
    case SizeHandleRect::Top:
    {
        qreal newH = qMax(20.0, m_localRect.bottom() - localPt.y());
        qreal dy   = m_height - newH;
        setPos(pos() + QPointF(0, dy));
        m_height = newH;
        break;
    }
    case SizeHandleRect::RightTop:
    {
        qreal newW = qMax(60.0, localPt.x() - m_localRect.left());
        qreal newH = qMax(20.0, m_localRect.bottom() - localPt.y());
        qreal dy   = m_height - newH;
        setPos(pos() + QPointF(0, dy));
        m_width  = newW;
        m_height = newH;
        break;
    }
    case SizeHandleRect::Right:
        m_width = qMax(60.0, localPt.x() - m_localRect.left());
        break;
    case SizeHandleRect::RightBottom:
        m_width  = qMax(60.0, localPt.x() - m_localRect.left());
        m_height = qMax(20.0, localPt.y() - m_localRect.top());
        break;
    case SizeHandleRect::Bottom:
        m_height = qMax(20.0, localPt.y() - m_localRect.top());
        break;
    case SizeHandleRect::LeftBottom:
    {
        qreal newW = qMax(60.0, m_localRect.right() - localPt.x());
        qreal dx   = m_width - newW;
        setPos(pos() + QPointF(dx, 0));
        m_width  = newW;
        m_height = qMax(20.0, localPt.y() - m_localRect.top());
        break;
    }
    case SizeHandleRect::Left:
    {
        qreal newW = qMax(60.0, m_localRect.right() - localPt.x());
        qreal dx   = m_width - newW;
        setPos(pos() + QPointF(dx, 0));
        m_width = newW;
        break;
    }
    default:
        break;
    }

    recalcLocalRect();
    updateHandles();
    update();
}
*/
// ===========================================================================
// Helpers shared between paint() and image()
// ===========================================================================

static bool textItemSideways(int rot)
{
    return rot == 90 || rot == 270;
}

QVector<GraphicsTextItem::LineInfo> GraphicsTextItem::buildLines() const
{
    QFontMetricsF fm(m_font);
    const qreal lineH   = fm.height();
    const qreal pad     = 4.0;
    const bool  sw      = textItemSideways(m_textRotation);
    const qreal availH  = sw ? m_width  : m_height;
    const qreal availW  = sw ? m_height : m_width;
    const qreal layoutW = availW / m_scaleX;

    const QStringList parts = m_text.split('\n');
    QVector<LineInfo> result;

    if (!m_wordWrap) {
        const qreal totalH = parts.size() * lineH;
        qreal startY = pad;
        if (m_alignment & Qt::AlignVCenter)
            startY = (availH - totalH) / 2.0;
        else if (m_alignment & Qt::AlignBottom)
            startY = availH - totalH - pad;

        int offset = 0;
        qreal y = startY;
        for (const QString &part : parts) {
            LineInfo li; li.text = part; li.textStart = offset; li.y = y;
            result.append(li);
            offset += part.length() + 1;
            y      += lineH;
        }
    } else {
        // Two-pass word-wrap
        int totalVLines = 0;
        for (const QString &part : parts) {
            if (part.isEmpty()) { ++totalVLines; continue; }
            QTextLayout lay; lay.setFont(m_font); lay.setText(part);
            QTextOption opt; opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            lay.setTextOption(opt);
            lay.beginLayout();
            int cnt = 0;
            while (true) {
                QTextLine tl = lay.createLine();
                if (!tl.isValid()) break;
                tl.setLineWidth(layoutW); ++cnt;
            }
            lay.endLayout();
            totalVLines += qMax(1, cnt);
        }

        const qreal totalH = totalVLines * lineH;
        qreal startY = pad;
        if (m_alignment & Qt::AlignVCenter) startY = (availH - totalH) / 2.0;
        else if (m_alignment & Qt::AlignBottom) startY = availH - totalH - pad;

        int offset = 0;
        qreal y = startY;
        for (const QString &part : parts) {
            if (part.isEmpty()) {
                LineInfo li; li.text = ""; li.textStart = offset; li.y = y;
                result.append(li); y += lineH; offset += 1; continue;
            }
            QTextLayout lay; lay.setFont(m_font); lay.setText(part);
            QTextOption opt;
            opt.setAlignment(m_alignment & Qt::AlignHorizontal_Mask);
            opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            lay.setTextOption(opt);
            lay.beginLayout();
            int firstLine = result.size();
            while (true) {
                QTextLine tl = lay.createLine();
                if (!tl.isValid()) break;
                tl.setLineWidth(layoutW);
                tl.setPosition(QPointF(0, y));
                LineInfo li;
                li.text      = part.mid(tl.textStart(), tl.textLength());
                li.textStart = offset + tl.textStart();
                li.y         = y;
                result.append(li);
                y += lineH;
            }
            lay.endLayout();
            if (result.size() == firstLine) {
                LineInfo li; li.text = ""; li.textStart = offset; li.y = y;
                result.append(li); y += lineH;
            }
            offset += part.length() + 1;
        }
    }
    return result;
}

bool GraphicsTextItem::isTextOverflowing() const
{
    QFontMetricsF fm(m_font);
    const QVector<LineInfo> lines = buildLines();
    const bool sw     = textItemSideways(m_textRotation);
    const qreal checkH = sw ? m_width  : m_height;
    const qreal checkW = sw ? m_height : m_width;

    if (!lines.isEmpty()) {
        const LineInfo &last = lines.last();
        if (last.y + fm.height() > checkH) return true;
    }
    if (!m_wordWrap) {
        for (const LineInfo &li : lines)
            if (fm.width(li.text) * m_scaleX > checkW) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Internal text drawing kernel — shared by paint() and image()
// ---------------------------------------------------------------------------
static void drawTextContent(QPainter *painter,
                            const QVector<GraphicsTextItem::LineInfo> &lines,
                            const QFont &font,
                            Qt::Alignment alignment,
                            qreal scaleX,
                            qreal textWidth,
                            qreal itemHeight,
                            int   textRotation,
                            bool  cursorVisible,
                            int   cursorPos,
                            bool  editing,
                            const QColor &textColor)
{
    const bool sw     = (textRotation == 90 || textRotation == 270);
    const qreal layoutW = sw ? itemHeight / scaleX : textWidth / scaleX;
    QFontMetricsF fm(font);

    painter->setClipRect(QRectF(0, 0, textWidth, itemHeight));
    painter->save();

    switch (textRotation) {
    case 90:  painter->translate(textWidth, 0);             painter->rotate(90);  break;
    case 180: painter->translate(textWidth, itemHeight);    painter->rotate(180); break;
    case 270: painter->translate(0, itemHeight);            painter->rotate(270); break;
    default: break;
    }
    painter->scale(scaleX, 1.0);

    for (const GraphicsTextItem::LineInfo &li : lines) {
        QTextLayout layout;
        layout.setFont(font);
        layout.setText(li.text.isEmpty() ? QString(" ") : li.text);
        QTextOption opt;
        opt.setAlignment(alignment & Qt::AlignHorizontal_Mask);
        opt.setWrapMode(QTextOption::NoWrap);
        layout.setTextOption(opt);
        layout.beginLayout();
        QTextLine tl = layout.createLine();
        if (tl.isValid()) {
            tl.setLineWidth(layoutW);
            tl.setPosition(QPointF(0, li.y));
        }
        layout.endLayout();

        painter->setPen(textColor);
        if (!li.text.isEmpty()) {
            QVector<QTextLayout::FormatRange> noSel;
            layout.draw(painter, QPointF(0, 0), noSel);
        }

        if (editing && cursorVisible) {
            const int lineEnd = li.textStart + li.text.length();
            if (cursorPos >= li.textStart && cursorPos <= lineEnd) {
                const int posInLine = cursorPos - li.textStart;
                if (li.text.isEmpty()) {
                    qreal cx = (alignment & Qt::AlignRight)   ? layoutW - 2 :
                               (alignment & Qt::AlignHCenter) ? layoutW / 2.0 : 2;
                    painter->setPen(QPen(QColor(30, 120, 255), 2));
                    painter->drawLine(QPointF(cx, li.y), QPointF(cx, li.y + fm.height()));
                } else {
                    layout.drawCursor(painter, QPointF(0, 0), posInLine, 2);
                }
            }
        }
    }

    painter->restore();
    painter->setClipping(false);
}

// ---------------------------------------------------------------------------
// paint()
// ---------------------------------------------------------------------------
void GraphicsTextItem::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *)
{
    Q_UNUSED(option)
    painter->save();

    const bool overflow  = isTextOverflowing();
    const bool selected  = isSelected();

    // --- Out-of-bounds check ---
    bool outOfBounds = false;
    // Access canvas rect via scene if EditorScene is available
    // (falls back gracefully if scene() is not an EditorScene)
    if (scene()) {
        // Try dynamic cast – works if EditorScene is in the build
        // Otherwise comment out and remove the EditorScene include
#ifdef EDITOR_SCENE_H
        EditorScene *es = qobject_cast<EditorScene *>(scene());
        if (es) {
            const QRectF canvas    = es->canvasRect();
            const QRectF itemScene = mapToScene(QRectF(0, 0, m_width, m_height))
                                         .boundingRect();
            outOfBounds = !canvas.contains(itemScene);
        }
#endif
    }

    // --- Background ---
    painter->setBrush(outOfBounds ? QColor(220, 50, 50, 80)
                                  : QColor(255, 255, 255, 220));
    QPen borderPen;
    if (m_editing)
        borderPen = QPen(QColor(30, 120, 255), 2, Qt::SolidLine);
    else if (selected)
        borderPen = QPen(QColor(80, 80, 200), 1.5, Qt::DashLine);
    else
        borderPen = QPen(QColor(160, 160, 160), 1, Qt::SolidLine);
    painter->setPen(borderPen);
    painter->drawRect(QRectF(0, 0, m_width, m_height));

    // --- Placeholder ---
    if (m_text.isEmpty() && !m_editing) {
        QFont pf = m_font; pf.setItalic(true);
        painter->setFont(pf);
        painter->setPen(QColor(180, 180, 180));
        painter->drawText(QRectF(4, 4, m_width - 8, m_height - 8),
                          Qt::AlignLeft | Qt::AlignTop, "Double-click to edit");
        painter->restore();
        return;
    }

    // --- Text ---
    const QColor textColor = overflow ? QColor(220, 30, 30) : QColor(20, 20, 20);
    drawTextContent(painter, buildLines(), m_font, m_alignment, m_scaleX,
                    m_width, m_height, m_textRotation,
                    m_cursorVisible, m_cursorPos, m_editing, textColor);

    // --- Per-side borders ---
    if (m_borderSides != 0) {
        QPen bp(QColor(20, 20, 20), m_borderWidth, Qt::SolidLine);
        bp.setCapStyle(Qt::FlatCap);
        painter->setPen(bp);
        const qreal w = m_width, h = m_height, half = m_borderWidth / 2.0;
        if (m_borderSides & BorderTop)    painter->drawLine(QPointF(0,half),    QPointF(w,half));
        if (m_borderSides & BorderBottom) painter->drawLine(QPointF(0,h-half),  QPointF(w,h-half));
        if (m_borderSides & BorderLeft)   painter->drawLine(QPointF(half,0),    QPointF(half,h));
        if (m_borderSides & BorderRight)  painter->drawLine(QPointF(w-half,0),  QPointF(w-half,h));
    }

    // NOTE: qdraw draws its own SizeHandleRect children automatically as child
    // items — we do NOT need to draw handles manually here.

    painter->restore();
}

// ---------------------------------------------------------------------------
// image() — clean render without interactive chrome
// ---------------------------------------------------------------------------
QImage GraphicsTextItem::image() const
{
    const int imgW = qMax(1, qRound(m_width));
    const int imgH = qMax(1, qRound(m_height));
    QImage img(imgW, imgH, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRectF(0, 0, m_width, m_height));

    const QColor textColor = isTextOverflowing() ? QColor(220, 30, 30)
                                                  : QColor(20, 20, 20);
    drawTextContent(&painter, buildLines(), m_font, m_alignment, m_scaleX,
                    m_width, m_height, m_textRotation,
                    false, 0, false, textColor);

    if (m_borderSides != 0) {
        QPen bp(QColor(20, 20, 20), m_borderWidth, Qt::SolidLine);
        bp.setCapStyle(Qt::FlatCap);
        painter.setPen(bp);
        const qreal w = m_width, h = m_height, half = m_borderWidth / 2.0;
        if (m_borderSides & BorderTop)    painter.drawLine(QPointF(0,half),   QPointF(w,half));
        if (m_borderSides & BorderBottom) painter.drawLine(QPointF(0,h-half), QPointF(w,h-half));
        if (m_borderSides & BorderLeft)   painter.drawLine(QPointF(half,0),   QPointF(half,h));
        if (m_borderSides & BorderRight)  painter.drawLine(QPointF(w-half,0), QPointF(w-half,h));
    }

    painter.end();
    return img;
}

// ---------------------------------------------------------------------------
// XML persistence (qdraw format)
// ---------------------------------------------------------------------------
bool GraphicsTextItem::saveToXml(QXmlStreamWriter *xml)
{
    xml->writeStartElement("textitem");
    xml->writeAttribute("x",           QString::number(pos().x()));
    xml->writeAttribute("y",           QString::number(pos().y()));
    xml->writeAttribute("width",       QString::number(m_width));
    xml->writeAttribute("height",      QString::number(m_height));
    xml->writeAttribute("text",        m_text);
    xml->writeAttribute("fontFamily",  m_font.family());
    xml->writeAttribute("fontSize",    QString::number(m_font.pointSize()));
    xml->writeAttribute("scaleX",      QString::number(m_scaleX));
    xml->writeAttribute("alignment",   QString::number(static_cast<int>(m_alignment)));
    xml->writeAttribute("rotation",    QString::number(m_textRotation));
    xml->writeAttribute("wordWrap",    m_wordWrap ? "1" : "0");
    xml->writeAttribute("locked",      m_locked   ? "1" : "0");
    xml->writeAttribute("borderSides", QString::number(m_borderSides));
    xml->writeAttribute("borderWidth", QString::number(m_borderWidth));
    xml->writeEndElement();
    return true;
}

bool GraphicsTextItem::loadFromXml(QXmlStreamReader *xml)
{
    const QXmlStreamAttributes attrs = xml->attributes();
    setPos(attrs.value("x").toDouble(), attrs.value("y").toDouble());
    m_width  = attrs.value("width").toDouble();
    m_height = attrs.value("height").toDouble();
    m_text       = attrs.value("text").toString();
    m_font.setFamily(attrs.value("fontFamily").toString());
    m_font.setPointSize(attrs.value("fontSize").toInt());
    m_scaleX       = attrs.value("scaleX").toDouble();
    m_alignment    = static_cast<Qt::Alignment>(attrs.value("alignment").toInt());
    m_textRotation = attrs.value("rotation").toInt();
    m_wordWrap     = attrs.value("wordWrap").toInt() != 0;
    m_locked       = attrs.value("locked").toInt()   != 0;
    m_borderSides  = attrs.value("borderSides").toInt();
    m_borderWidth  = attrs.value("borderWidth").toDouble();
    setLocked(m_locked);
    recalcLocalRect();
    updatehandles();
    xml->skipCurrentElement();
    return true;
}

// ---------------------------------------------------------------------------
// Property setters
// ---------------------------------------------------------------------------
void GraphicsTextItem::setText(const QString &text)
{
    prepareGeometryChange();
    m_text = text;
    m_cursorPos = qMin(m_cursorPos, text.length());
    update();
}
void GraphicsTextItem::setTextWidth(qreal w)
{
    prepareGeometryChange();
    m_width = qMax(60.0, w);
    recalcLocalRect(); updatehandles(); update();
}
void GraphicsTextItem::setItemHeight(qreal h)
{
    prepareGeometryChange();
    m_height = qMax(20.0, h);
    recalcLocalRect(); updatehandles(); update();
}
void GraphicsTextItem::setAlignment(Qt::Alignment a) { m_alignment = a; update(); }
void GraphicsTextItem::setFont(const QFont &f)
{
    prepareGeometryChange(); m_font = f; update();
}
void GraphicsTextItem::setScaleX(qreal sx) { m_scaleX = qMax(0.01, sx); update(); }
void GraphicsTextItem::setLocked(bool locked)
{
    m_locked = locked;
    setFlag(QGraphicsItem::ItemIsMovable,   !locked && !m_editing);
    // Hide/show handles by showing/hiding child SizeHandleRect items
    for (QGraphicsItem *child : childItems()) {
        if (child->type() == SizeHandleRect::Type)
            child->setVisible(!locked);
    }
    update();
}
void GraphicsTextItem::setTextRotation(int degrees)
{
    int a = degrees % 360;
    if (a < 0) a += 360;
    m_textRotation = (a / 90) * 90;
    update();
}
void GraphicsTextItem::setBorderSides(int sides) { m_borderSides = sides; update(); }
void GraphicsTextItem::setBorderWidth(qreal w)   { m_borderWidth = qMax(0.5, w); update(); }
void GraphicsTextItem::setWordWrap(bool wrap)    { m_wordWrap = wrap; update(); }

// ---------------------------------------------------------------------------
// Auto-fit font
// ---------------------------------------------------------------------------
void GraphicsTextItem::autoFitFont()
{
    if (m_text.isEmpty()) return;
    const QStringList lines = m_text.split('\n');
    const int   lineCount   = lines.size();
    const qreal pad         = 4.0;
    const bool  sw          = textItemSideways(m_textRotation);
    const qreal availH      = (sw ? m_width  : m_height) - pad * 2;
    const qreal availW      =  sw ? m_height : m_width;

    if (availH <= 0 || availW <= 0 || lineCount <= 0) return;

    QFont f = m_font;
    f.setStretch(100);

    int lo = 1, hi = 1000, bestSize = 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        f.setPointSize(mid);
        QFontMetricsF fm(f);
        if (fm.height() * lineCount <= availH) { bestSize = mid; lo = mid + 1; }
        else hi = mid - 1;
    }
    f.setPointSize(bestSize);

    QFontMetricsF fm(f);
    qreal maxW = 0;
    for (const QString &line : lines)
        maxW = qMax(maxW, fm.width(line));
    m_scaleX = (maxW > 0) ? (availW / maxW) * (1.0 - 1e-6) : 1.0;
    m_font   = f;
    update();
}

// ---------------------------------------------------------------------------
// getWrappedText
// ---------------------------------------------------------------------------
QString GraphicsTextItem::getWrappedText() const
{
    const QVector<LineInfo> lines = buildLines();
    QStringList parts;
    parts.reserve(lines.size());
    for (const LineInfo &li : lines)
        parts.append(li.text);
    return parts.join('\n');
}

// ---------------------------------------------------------------------------
// Editing
// ---------------------------------------------------------------------------
void GraphicsTextItem::setEditing(bool editing)
{
    m_editing = editing;
    if (editing) {
        m_cursorPos     = m_text.length();
        m_cursorVisible = true;
        m_cursorTimer.start(500);
        setFlag(QGraphicsItem::ItemIsMovable, false);
    } else {
        m_cursorTimer.stop();
        m_cursorVisible = false;
        setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
    }
    update();
}

void GraphicsTextItem::handleKeyPress(QKeyEvent *event)
{
    if (!m_editing) return;
    switch (event->key()) {
    case Qt::Key_Backspace:
        if (m_cursorPos > 0) { m_text.remove(--m_cursorPos, 1); } break;
    case Qt::Key_Delete:
        if (m_cursorPos < m_text.length()) m_text.remove(m_cursorPos, 1); break;
    case Qt::Key_Left:  if (m_cursorPos > 0)                m_cursorPos--; break;
    case Qt::Key_Right: if (m_cursorPos < m_text.length())  m_cursorPos++; break;
    case Qt::Key_Return: case Qt::Key_Enter:
        m_text.insert(m_cursorPos++, '\n'); break;
    case Qt::Key_Escape: setEditing(false); return;
    default:
        if (!event->text().isEmpty() && event->text()[0].isPrint()) {
            m_text.insert(m_cursorPos, event->text());
            m_cursorPos += event->text().length();
        }
        break;
    }
    m_cursorVisible = true;
    m_cursorTimer.start(500);
    update();
}

void GraphicsTextItem::handleMousePress(const QPointF &pos)
{
    if (!m_editing) return;
    const QVector<LineInfo> lines = buildLines();
    QFontMetricsF fm(m_font);
    const qreal lineH  = fm.height();
    const qreal naturalW = m_width / m_scaleX;

    int targetLine = lines.size() - 1;
    for (int i = 0; i < lines.size(); ++i) {
        if (pos.y() < lines[i].y + lineH) { targetLine = i; break; }
    }
    if (targetLine < 0 || targetLine >= lines.size()) {
        m_cursorPos = m_text.length(); update(); return;
    }
    const LineInfo &li = lines[targetLine];
    QTextLayout layout;
    layout.setFont(m_font);
    layout.setText(li.text.isEmpty() ? QString(" ") : li.text);
    QTextOption opt;
    opt.setAlignment(m_alignment & Qt::AlignHorizontal_Mask);
    opt.setWrapMode(QTextOption::NoWrap);
    layout.setTextOption(opt);
    layout.beginLayout();
    QTextLine tl = layout.createLine();
    if (tl.isValid()) { tl.setLineWidth(naturalW); tl.setPosition(QPointF(0, li.y)); }
    layout.endLayout();
    int posInLine = tl.isValid() ? tl.xToCursor(pos.x() / m_scaleX) : 0;
    posInLine   = qBound(0, posInLine, li.text.length());
    m_cursorPos = li.textStart + posInLine;
    m_cursorVisible = true;
    update();
}
