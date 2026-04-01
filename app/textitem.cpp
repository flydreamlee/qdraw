// ---------------------------------------------------------------------------
// GraphicsTextItem.cpp — qdraw-integrated text item implementation
// ---------------------------------------------------------------------------
#include "TextItem.h"
#include <QPainter>
#include <QFontMetricsF>
#include <QTextLayout>
#include <QTextOption>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QtMath>

// ===========================================================================
// Construction
// ===========================================================================
/*
GraphicsTextItem::GraphicsTextItem(QGraphicsItem *parent)
    : GraphicsItem(parent)   // creates 8 SizeHandleRect child handles
    , m_text("")
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

    // qdraw's DrawScene starts the draw gesture with stretch(RightBottom,0,0,origin)
    // applied to an item whose m_initialRect = QRect(0,0,1,1).
    // We therefore initialize to a 1×1 rect and let stretch()/updateCoordinate()
    // set the real size once the user completes the drag.
    m_width  = 1.0;
    m_height = 1.0;
    m_localRect   = QRectF(0, 0, 1, 1);
    m_initialRect = m_localRect;

    m_textWidth  = MIN_W;
    m_textHeight = MIN_H;

    // ItemIsFocusable is required for keyPressEvent to be delivered to this item
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    // Cursor blink timer
    QObject::connect(&m_cursorTimer, &QTimer::timeout, [this]() {
        m_cursorVisible = !m_cursorVisible;
        update();
    });

    updatehandles();
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
//m_originPoint = QPointF(0,0);

    // Set the qdraw local rect (used by GraphicsItem for handle positioning)
    //recalcLocalRect();

    // ItemIsFocusable is required for keyPressEvent to be delivered to this item
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    // Cursor blink
    QObject::connect(&m_cursorTimer, &QTimer::timeout, [this]() {
        m_cursorVisible = !m_cursorVisible;
        update();
    });

    updatehandles();
}

// ===========================================================================
// syncDimensions — keep m_textWidth/m_textHeight from m_width/m_height
// (m_width/m_height are owned by AbstractShapeType, our text dims mirror them)
// ===========================================================================

void GraphicsTextItem::syncDimensions()
{
    m_textWidth  = qMax(MIN_W, qAbs(m_width));
    m_textHeight = qMax(MIN_H, qAbs(m_height));
}

// ===========================================================================
// boundingRect — use m_localRect exactly as GraphicsRectItem does
// ===========================================================================

QRectF GraphicsTextItem::boundingRect() const
{
    return m_localRect;
}

// ===========================================================================
// updatehandles — reposition the 8 SizeHandleRect children from m_localRect
// ===========================================================================

void GraphicsTextItem::updatehandles()
{
    // Delegate to GraphicsItem::updatehandles() which positions handles
    // from boundingRect() == m_localRect
    GraphicsItem::updatehandles();
}

// ===========================================================================
// stretch — called by DrawScene during the mouse-draw gesture
//
// Follows the same pattern as GraphicsRectItem::stretch():
//   apply QTransform::scale to m_initialRect → m_localRect
// ===========================================================================

void GraphicsTextItem::stretch(int handle, double sx, double sy,
                               const QPointF &origin)
{
    // For edge-only handles, lock the orthogonal axis
    switch (handle) {
    case Right:
    case Left:
        sy = 1.0;
        break;
    case Top:
    case Bottom:
        sx = 1.0;
        break;
    default:
        break;
    }

    QTransform trans;
    trans.translate(origin.x(), origin.y());
    trans.scale(sx, sy);
    trans.translate(-origin.x(), -origin.y());

    prepareGeometryChange();
    m_localRect = trans.mapRect(m_initialRect);
    m_width     = m_localRect.width();
    m_height    = m_localRect.height();
    syncDimensions();
    updatehandles();
}

// ===========================================================================
// updateCoordinate — called by DrawScene on mouseReleaseEvent
//
// Normalizes the item: centers m_localRect at (-w/2, -h/2) and adjusts
// pos() + transformOriginPoint() so the item appears in the same place.
// Mirrors GraphicsRectItem::updateCoordinate() exactly.
// ===========================================================================

void GraphicsTextItem::updateCoordinate()
{
    QPointF pt1, pt2, delta;
    pt1   = mapToScene(transformOriginPoint());
    pt2   = mapToScene(m_localRect.center());
    delta = pt1 - pt2;

    if (!parentItem()) {
        prepareGeometryChange();

        m_width  = qAbs(m_localRect.width());
        m_height = qAbs(m_localRect.height());
        syncDimensions();

        m_localRect = QRectF(-m_width / 2.0, -m_height / 2.0, m_width, m_height);

        setTransform(transform().translate(delta.x(), delta.y()));
        setTransformOriginPoint(m_localRect.center());
        moveBy(-delta.x(), -delta.y());
        setTransform(transform().translate(-delta.x(), -delta.y()));

        updatehandles();
    }

    m_initialRect = m_localRect;
}

// ===========================================================================
// resizeTo — called when user drags a SizeHandleRect on an existing item
//
// Unlike stretch(), this works in scene coordinates and directly sets the
// item's width/height + pos().
// ===========================================================================

//void GraphicsTextItem::resizeTo(SizeHandleRect::Direction dir, const QPointF &scenePoint)
void GraphicsTextItem::resizeTo(int dir, const QPointF &scenePoint)
{
    // Convert to item-local coordinates (relative to item origin)
    QPointF localPt = mapFromScene(scenePoint);

    prepareGeometryChange();

    const qreal l = m_localRect.left();
    const qreal t = m_localRect.top();
    const qreal r = m_localRect.right();
    const qreal b = m_localRect.bottom();

    switch (dir) {
    case LeftTop:
    {
        qreal newW = qMax(MIN_W, r - localPt.x());
        qreal newH = qMax(MIN_H, b - localPt.y());
        QPointF d(m_width - newW, m_height - newH);
        setPos(pos() + d);
        m_localRect.setLeft(r - newW);
        m_localRect.setTop(b - newH);
        break;
    }
    case Top:
    {
        qreal newH = qMax(MIN_H, b - localPt.y());
        qreal dy   = m_height - newH;
        setPos(pos() + QPointF(0, dy));
        m_localRect.setTop(b - newH);
        break;
    }
    case RightTop:
    {
        qreal newW = qMax(MIN_W, localPt.x() - l);
        qreal newH = qMax(MIN_H, b - localPt.y());
        qreal dy   = m_height - newH;
        setPos(pos() + QPointF(0, dy));
        m_localRect.setRight(l + newW);
        m_localRect.setTop(b - newH);
        break;
    }
    case Right:
        m_localRect.setRight(qMax(l + MIN_W, localPt.x()));
        break;
    case RightBottom:
        m_localRect.setRight(qMax(l + MIN_W,  localPt.x()));
        m_localRect.setBottom(qMax(t + MIN_H, localPt.y()));
        break;
    case Bottom:
        m_localRect.setBottom(qMax(t + MIN_H, localPt.y()));
        break;
    case LeftBottom:
    {
        qreal newW = qMax(MIN_W, r - localPt.x());
        qreal dx   = m_width - newW;
        setPos(pos() + QPointF(dx, 0));
        m_localRect.setLeft(r - newW);
        m_localRect.setBottom(qMax(t + MIN_H, localPt.y()));
        break;
    }
    case Left:
    {
        qreal newW = qMax(MIN_W, r - localPt.x());
        qreal dx   = m_width - newW;
        setPos(pos() + QPointF(dx, 0));
        m_localRect.setLeft(r - newW);
        break;
    }
    default:
        break;
    }

    m_width  = m_localRect.width();
    m_height = m_localRect.height();
    syncDimensions();
    m_initialRect = m_localRect;
    updatehandles();
    update();
}

// ===========================================================================
// buildLines — text layout helper (word-wrap aware)
// ===========================================================================

static bool gtSideways(int rot) { return rot == 90 || rot == 270; }

QVector<GraphicsTextItem::LineInfo> GraphicsTextItem::buildLines() const
{
    QFontMetricsF fm(m_font);
    const qreal lineH  = fm.height();
    const qreal pad    = 4.0;
    const bool  sw     = gtSideways(m_textRotation);
    const qreal availH = sw ? m_textWidth  : m_textHeight;
    const qreal availW = sw ? m_textHeight : m_textWidth;
    const qreal layW   = availW / m_scaleX;

    const QStringList parts = m_text.split('\n');
    QVector<LineInfo> result;

    if (!m_wordWrap) {
        const qreal totalH = parts.size() * lineH;
        qreal startY = pad;
        if (m_alignment & Qt::AlignVCenter)  startY = (availH - totalH) / 2.0;
        else if (m_alignment & Qt::AlignBottom) startY = availH - totalH - pad;

        int offset = 0;
        qreal y = startY;
        for (const QString &part : parts) {
            result.append({part, offset, y});
            offset += part.length() + 1;
            y      += lineH;
        }
    } else {
        // Two-pass word-wrap
        int totalVL = 0;
        for (const QString &part : parts) {
            if (part.isEmpty()) { ++totalVL; continue; }
            QTextLayout lay; lay.setFont(m_font); lay.setText(part);
            QTextOption opt;
            opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            lay.setTextOption(opt);
            lay.beginLayout();
            int cnt = 0;
            while (true) {
                QTextLine tl = lay.createLine();
                if (!tl.isValid()) break;
                tl.setLineWidth(layW); ++cnt;
            }
            lay.endLayout();
            totalVL += qMax(1, cnt);
        }
        const qreal totalH = totalVL * lineH;
        qreal startY = pad;
        if (m_alignment & Qt::AlignVCenter)  startY = (availH - totalH) / 2.0;
        else if (m_alignment & Qt::AlignBottom) startY = availH - totalH - pad;

        int offset = 0;
        qreal y = startY;
        for (const QString &part : parts) {
            if (part.isEmpty()) {
                result.append({"", offset, y}); y += lineH; offset += 1; continue;
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
                tl.setLineWidth(layW);
                tl.setPosition(QPointF(0, y));
                result.append({part.mid(tl.textStart(), tl.textLength()),
                                offset + tl.textStart(), y});
                y += lineH;
            }
            lay.endLayout();
            if (result.size() == firstLine) {
                result.append({"", offset, y}); y += lineH;
            }
            offset += part.length() + 1;
        }
    }
    return result;
}

bool GraphicsTextItem::isTextOverflowing() const
{
    QFontMetricsF fm(m_font);
    const auto lines = buildLines();
    const bool sw    = gtSideways(m_textRotation);
    const qreal chkH = sw ? m_textWidth  : m_textHeight;
    const qreal chkW = sw ? m_textHeight : m_textWidth;
    if (!lines.isEmpty()) {
        if (lines.last().y + fm.height() > chkH) return true;
    }
    if (!m_wordWrap) {
        for (const auto &li : lines)
            if (fm.width(li.text) * m_scaleX > chkW) return true;
    }
    return false;
}

// ===========================================================================
// paint
// ===========================================================================

void GraphicsTextItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *)
{
    Q_UNUSED(option)
    painter->save();

    // Draw rect uses m_localRect (which may be negative-origin after updateCoordinate)
    const QRectF drawRect = m_localRect;

    // Background
    const bool overflow = isTextOverflowing();
    painter->setBrush(overflow ? QColor(220, 50, 50, 80)
                               : QColor(255, 255, 255, 220));

    QPen borderPen;
    if (m_editing)
        borderPen = QPen(QColor(30, 120, 255), 2);
    else if (isSelected())
        borderPen = QPen(QColor(80, 80, 200), 1.5, Qt::DashLine);
    else
        borderPen = QPen(QColor(160, 160, 160), 1);
    painter->setPen(borderPen);
    painter->drawRect(drawRect);

    // Placeholder
    if (m_text.isEmpty() && !m_editing) {
        QFont pf = m_font; pf.setItalic(true);
        painter->setFont(pf);
        painter->setPen(QColor(180, 180, 180));
        painter->drawText(drawRect.adjusted(4, 4, -4, -4),
                          Qt::AlignLeft | Qt::AlignTop, "Double-click to edit");
        painter->restore();
        return;
    }

    // Text drawing
    const bool sw      = gtSideways(m_textRotation);
    const qreal layW   = (sw ? m_textHeight : m_textWidth) / m_scaleX;
    const QColor txtC  = overflow ? QColor(220, 30, 30) : QColor(20, 20, 20);
    const auto lines   = buildLines();
    QFontMetricsF fm(m_font);

    painter->setClipRect(drawRect);
    painter->save();

    // Translate to drawRect origin before applying rotation + scale
    painter->translate(drawRect.topLeft());
    switch (m_textRotation) {
    case 90:  painter->translate(m_textWidth, 0);              painter->rotate(90);  break;
    case 180: painter->translate(m_textWidth, m_textHeight);   painter->rotate(180); break;
    case 270: painter->translate(0, m_textHeight);             painter->rotate(270); break;
    default:  break;
    }
    painter->scale(m_scaleX, 1.0);

    for (const LineInfo &li : lines) {
        QTextLayout layout;
        layout.setFont(m_font);
        layout.setText(li.text.isEmpty() ? QString(" ") : li.text);
        QTextOption opt;
        opt.setAlignment(m_alignment & Qt::AlignHorizontal_Mask);
        opt.setWrapMode(QTextOption::NoWrap);
        layout.setTextOption(opt);
        layout.beginLayout();
        QTextLine tl = layout.createLine();
        if (tl.isValid()) {
            tl.setLineWidth(layW);
            tl.setPosition(QPointF(0, li.y));
        }
        layout.endLayout();

        painter->setPen(txtC);
        if (!li.text.isEmpty()) {
            QVector<QTextLayout::FormatRange> noSel;
            layout.draw(painter, QPointF(0, 0), noSel);
        }

        // Cursor
        if (m_editing && m_cursorVisible) {
            const int lineEnd = li.textStart + li.text.length();
            if (m_cursorPos >= li.textStart && m_cursorPos <= lineEnd) {
                const int posInLine = m_cursorPos - li.textStart;
                if (li.text.isEmpty()) {
                    qreal cx = (m_alignment & Qt::AlignRight)   ? layW - 2 :
                               (m_alignment & Qt::AlignHCenter) ? layW / 2.0 : 2;
                    painter->setPen(QPen(QColor(30, 120, 255), 2));
                    painter->drawLine(QPointF(cx, li.y),
                                      QPointF(cx, li.y + fm.height()));
                } else {
                    layout.drawCursor(painter, QPointF(0, 0), posInLine, 2);
                }
            }
        }
    }
    painter->restore();
    painter->setClipping(false);

    // Per-side borders
    if (m_borderSides != 0) {
        QPen bp(QColor(20, 20, 20), m_borderWidth, Qt::SolidLine);
        bp.setCapStyle(Qt::FlatCap);
        painter->setPen(bp);
        const qreal half = m_borderWidth / 2.0;
        const qreal l = drawRect.left(), t = drawRect.top(),
                    r = drawRect.right(), b = drawRect.bottom();
        if (m_borderSides & BorderTop)    painter->drawLine(QPointF(l,t+half),  QPointF(r,t+half));
        if (m_borderSides & BorderBottom) painter->drawLine(QPointF(l,b-half),  QPointF(r,b-half));
        if (m_borderSides & BorderLeft)   painter->drawLine(QPointF(l+half,t),  QPointF(l+half,b));
        if (m_borderSides & BorderRight)  painter->drawLine(QPointF(r-half,t),  QPointF(r-half,b));
    }

    painter->restore();
}

// ===========================================================================
// toImage
// ===========================================================================

QImage GraphicsTextItem::toImage() const
{
    const int w = qMax(1, qRound(m_textWidth));
    const int h = qMax(1, qRound(m_textHeight));
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    p.setBrush(Qt::white); p.setPen(Qt::NoPen);
    p.drawRect(QRectF(0, 0, w, h));

    const bool sw    = gtSideways(m_textRotation);
    const qreal layW = (sw ? m_textHeight : m_textWidth) / m_scaleX;
    const QColor txtC = isTextOverflowing() ? QColor(220,30,30) : QColor(20,20,20);
    const auto lines  = buildLines();
    QFontMetricsF fm(m_font);

    p.setClipRect(QRectF(0, 0, w, h));
    p.save();
    switch (m_textRotation) {
    case 90:  p.translate(w, 0); p.rotate(90);  break;
    case 180: p.translate(w, h); p.rotate(180); break;
    case 270: p.translate(0, h); p.rotate(270); break;
    default: break;
    }
    p.scale(m_scaleX, 1.0);

    for (const LineInfo &li : lines) {
        if (li.text.isEmpty()) continue;
        QTextLayout layout; layout.setFont(m_font); layout.setText(li.text);
        QTextOption opt;
        opt.setAlignment(m_alignment & Qt::AlignHorizontal_Mask);
        opt.setWrapMode(QTextOption::NoWrap);
        layout.setTextOption(opt);
        layout.beginLayout();
        QTextLine tl = layout.createLine();
        if (tl.isValid()) { tl.setLineWidth(layW); tl.setPosition(QPointF(0, li.y)); }
        layout.endLayout();
        p.setPen(txtC);
        QVector<QTextLayout::FormatRange> ns;
        layout.draw(&p, QPointF(0,0), ns);
    }
    p.restore(); p.setClipping(false);

    if (m_borderSides) {
        QPen bp(QColor(20,20,20), m_borderWidth, Qt::SolidLine);
        bp.setCapStyle(Qt::FlatCap); p.setPen(bp);
        const qreal half = m_borderWidth / 2.0;
        if (m_borderSides & BorderTop)    p.drawLine(QPointF(0,half),    QPointF(w,half));
        if (m_borderSides & BorderBottom) p.drawLine(QPointF(0,h-half),  QPointF(w,h-half));
        if (m_borderSides & BorderLeft)   p.drawLine(QPointF(half,0),    QPointF(half,h));
        if (m_borderSides & BorderRight)  p.drawLine(QPointF(w-half,0),  QPointF(w-half,h));
    }
    p.end();
    return img;
}

// ===========================================================================
// duplicate
// ===========================================================================

QGraphicsItem *GraphicsTextItem::duplicate() const
{
    GraphicsTextItem *item = new GraphicsTextItem(m_localRect);
    item->m_text           = m_text;
    item->m_font           = m_font;
    item->m_alignment      = m_alignment;
    item->m_scaleX         = m_scaleX;
    item->m_locked     = m_locked;
    item->m_textRotation   = m_textRotation;
    item->m_wordWrap       = m_wordWrap;
    item->m_borderSides    = m_borderSides;
    item->m_borderWidth= m_borderWidth;
    item->m_width          = m_width;
    item->m_height         = m_height;
    item->m_localRect      = m_localRect;
    item->m_initialRect    = m_initialRect;
    item->syncDimensions();
    item->setPos(pos());
    item->setPen(pen());
    item->setBrush(brush());
    item->setTransform(transform());
    item->setTransformOriginPoint(transformOriginPoint());
    item->setRotation(rotation());
    item->setScale(scale());
    item->setZValue(zValue() + 0.1);
    item->updateCoordinate();
    return item;
}

// ===========================================================================
// XML
// ===========================================================================

bool GraphicsTextItem::saveToXml(QXmlStreamWriter *xml)
{
    xml->writeStartElement("textitem");
    writeBaseAttributes(xml);   // x, y, z, width, height, rotate (from GraphicsItem)
    xml->writeAttribute("text",        m_text);
    xml->writeAttribute("fontFamily",  m_font.family());
    xml->writeAttribute("fontSize",    QString::number(m_font.pointSize()));
    xml->writeAttribute("scaleX",      QString::number(m_scaleX));
    xml->writeAttribute("alignment",   QString::number(static_cast<int>(m_alignment)));
    xml->writeAttribute("textRotation",QString::number(m_textRotation));
    xml->writeAttribute("wordWrap",    m_wordWrap    ? "1" : "0");
    xml->writeAttribute("locked",      m_locked  ? "1" : "0");
    xml->writeAttribute("borderSides", QString::number(m_borderSides));
    xml->writeAttribute("borderWidth", QString::number(m_borderWidth));
    xml->writeEndElement();
    return true;
}

bool GraphicsTextItem::loadFromXml(QXmlStreamReader *xml)
{
    readBaseAttributes(xml);    // sets pos(), m_width, m_height (from GraphicsItem)
    const QXmlStreamAttributes a = xml->attributes();
    m_text          = a.value("text").toString();
    m_font.setFamily(a.value("fontFamily").toString());
    m_font.setPointSize(a.value("fontSize").toInt());
    m_scaleX        = a.value("scaleX").toDouble();
    if (m_scaleX <= 0) m_scaleX = 1.0;
    m_alignment     = static_cast<Qt::Alignment>(a.value("alignment").toInt());
    m_textRotation  = a.value("textRotation").toInt();
    m_wordWrap      = a.value("wordWrap").toInt() != 0;
    m_locked    = a.value("locked").toInt()   != 0;
    m_borderSides   = a.value("borderSides").toInt();
    m_borderWidth = a.value("borderWidth").toDouble();
    if (m_borderWidth <= 0) m_borderWidth = 2.0;
    syncDimensions();
    updateCoordinate();
    xml->skipCurrentElement();
    return true;
}

// ===========================================================================
// Setters
// ===========================================================================

void GraphicsTextItem::setText(const QString &t)
{
    m_text = t;
    m_cursorPos = qMin(m_cursorPos, t.length());
    update();
}

void GraphicsTextItem::setAlignment(Qt::Alignment a) { m_alignment = a; update(); }

void GraphicsTextItem::setFont(const QFont &f)
{
    prepareGeometryChange(); m_font = f; update();
}

void GraphicsTextItem::setScaleX(qreal sx) { m_scaleX = qMax(0.01, sx); update(); }

void GraphicsTextItem::setTextLocked(bool locked)
{
    m_locked = locked;
    setFlag(QGraphicsItem::ItemIsMovable, !locked && !m_editing);
    for (QGraphicsItem *c : childItems())
        if (c->type() == SizeHandleRect::Type) c->setVisible(!locked);
    update();
}

void GraphicsTextItem::setTextRotation(int deg)
{
    int a = deg % 360; if (a < 0) a += 360;
    m_textRotation = (a / 90) * 90; update();
}

void GraphicsTextItem::setBorderSides(int s) { m_borderSides = s; update(); }
void GraphicsTextItem::setBorderLineWidth(qreal w) { m_borderWidth = qMax(0.5,w); update(); }
void GraphicsTextItem::setWordWrap(bool wrap) { m_wordWrap = wrap; update(); }

// ===========================================================================
// Auto-fit font
// ===========================================================================

void GraphicsTextItem::autoFitFont()
{
    if (m_text.isEmpty()) return;
    const QStringList lines = m_text.split('\n');
    const int   lc  = lines.size();
    const qreal pad = 4.0;
    const bool  sw  = gtSideways(m_textRotation);
    const qreal aH  = (sw ? m_textWidth  : m_textHeight) - pad * 2;
    const qreal aW  =  sw ? m_textHeight : m_textWidth;
    if (aH <= 0 || aW <= 0 || lc <= 0) return;

    QFont f = m_font; f.setStretch(100);
    int lo = 1, hi = 1000, best = 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        f.setPointSize(mid);
        QFontMetricsF fm(f);
        if (fm.height() * lc <= aH) { best = mid; lo = mid + 1; }
        else hi = mid - 1;
    }
    f.setPointSize(best);
    QFontMetricsF fm(f);
    qreal maxW = 0;
    for (const QString &l : lines) maxW = qMax(maxW, fm.width(l));
    m_scaleX = (maxW > 0) ? (aW / maxW) * (1.0 - 1e-6) : 1.0;
    m_font   = f;
    update();
}

// ===========================================================================
// getWrappedText
// ===========================================================================

QString GraphicsTextItem::getWrappedText() const
{
    const auto lines = buildLines();
    QStringList parts; parts.reserve(lines.size());
    for (const auto &li : lines) parts.append(li.text);
    return parts.join('\n');
}

// ===========================================================================
// Editing
// ===========================================================================

void GraphicsTextItem::setEditing(bool editing)
{
    m_editing = editing;
    if (editing) {
        m_cursorPos     = m_text.length();
        m_cursorVisible = true;
        m_cursorTimer.start(500);
        setFlag(QGraphicsItem::ItemIsMovable, false);
        // Grab keyboard focus so keyPressEvent is delivered here
        if (!hasFocus())
            setFocus(Qt::OtherFocusReason);
    } else {
        m_cursorTimer.stop();
        m_cursorVisible = false;
        setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
        // Release focus so normal scene key handling resumes
        if (hasFocus())
            clearFocus();
    }
    update();
}

void GraphicsTextItem::handleMousePress(const QPointF &pos)
{
    if (!m_editing) return;
    const auto lines   = buildLines();
    QFontMetricsF fm(m_font);
    const qreal lineH  = fm.height();
    const qreal layW   = m_textWidth / m_scaleX;

    int targetLine = lines.size() - 1;
    for (int i = 0; i < lines.size(); ++i)
        if (pos.y() < lines[i].y + lineH) { targetLine = i; break; }

    if (targetLine < 0 || targetLine >= lines.size()) {
        m_cursorPos = m_text.length(); update(); return;
    }
    const LineInfo &li = lines[targetLine];
    QTextLayout layout; layout.setFont(m_font);
    layout.setText(li.text.isEmpty() ? QString(" ") : li.text);
    QTextOption opt;
    opt.setAlignment(m_alignment & Qt::AlignHorizontal_Mask);
    opt.setWrapMode(QTextOption::NoWrap);
    layout.setTextOption(opt);
    layout.beginLayout();
    QTextLine tl = layout.createLine();
    if (tl.isValid()) { tl.setLineWidth(layW); tl.setPosition(QPointF(0, li.y)); }
    layout.endLayout();
    int pil = tl.isValid() ? tl.xToCursor(pos.x() / m_scaleX) : 0;
    pil = qBound(0, pil, li.text.length());
    m_cursorPos = li.textStart + pil;
    m_cursorVisible = true;
    update();
}

// ===========================================================================
// Qt event handlers for text input
// ===========================================================================

void GraphicsTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setEditing(true);
        // Place cursor at click position
        handleMousePress(mapFromScene(event->scenePos()));
        // Grab focus so keyPressEvent is delivered to this item
        setFocus(Qt::MouseFocusReason);
        event->accept();
        return;
    }
    GraphicsItem::mouseDoubleClickEvent(event);
}

void GraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_editing && event->button() == Qt::LeftButton) {
        // Place cursor at click position while already editing
        handleMousePress(mapFromScene(event->scenePos()));
        event->accept();
        return;
    }
    // Not editing — let qdraw's base class handle selection / move / resize
    GraphicsItem::mousePressEvent(event);
}

void GraphicsTextItem::keyPressEvent(QKeyEvent *event)
{
    if (m_editing) {
        // Escape exits editing without consuming; all other keys go to text
        if (event->key() == Qt::Key_Escape) {
            setEditing(false);
            clearFocus();
            event->accept();
            return;
        }
        // Delegate to our internal key handler
        if (!m_locked) {
                switch (event->key()) {
            case Qt::Key_Backspace:
                if (m_cursorPos > 0) m_text.remove(--m_cursorPos, 1); break;
            case Qt::Key_Delete:
                if (m_cursorPos < m_text.length()) m_text.remove(m_cursorPos, 1); break;
            case Qt::Key_Left:  if (m_cursorPos > 0)               m_cursorPos--; break;
            case Qt::Key_Right: if (m_cursorPos < m_text.length()) m_cursorPos++; break;
            case Qt::Key_Return: case Qt::Key_Enter:
                m_text.insert(m_cursorPos++, '\n'); break;
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
        event->accept();
        return;
    }
    // Not editing — pass to base (handles Delete key for item removal, etc.)
    GraphicsItem::keyPressEvent(event);
}

void GraphicsTextItem::focusOutEvent(QFocusEvent *event)
{
    // Exit text editing when the item loses focus (e.g. user clicks elsewhere)
    if (m_editing)
        setEditing(false);
    GraphicsItem::focusOutEvent(event);
}
