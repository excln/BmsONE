#ifndef MASTERVIEW_H
#define MASTERVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "SoundChannelDef.h"
#include "ScalarRegion.h"

class SequenceView;
class Document;
class MasterCache;
class AudioPlaySource;
class MasterPlayer;


class MiniMapView : public QWidget
{
	Q_OBJECT

	friend class MasterLaneView;

private:
	struct RmsCachePacket{
		Rms rms;
		Rms peak;
		bool available;
	};

private:
	SequenceView *sview;
	Document *document;
	MasterCache *master;
	int posX, posWidth;
	int posTop, posHeight;
	bool present;
	bool fixed;
	double opacity;

	bool wholeRmsCacheInvalid;
	ScalarRegion rmsCacheInvalidRegions;
	QVector<RmsCachePacket> rmsCacheOfTicks;

	static const int BufferWidth = 88, BufferHeight = 1024;
	bool bufferInvalid;
	QImage buffer;

	QTimer timer;

	bool dragging;
	int dragYOrigin;
	int dragVOrigin;

	void ReconstructRmsCache();
	void UpdateRmsCachePartially();
	void UpdateBuffer();

private slots:
	void PopOutAnimationFinished();
	void MasterCacheCleared();
	void MasterCacheUpdated(int position, int length);
	void OnTimer();

	void MiniMapOpacityChanged(double value);

signals:
	void RmsCacheUpdated();

public:
	MiniMapView(SequenceView *sview);
	~MiniMapView();

	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

	void ReplaceDocument(Document *newDocument);

	void SetPosition(int x, int top, int height);
	bool IsPresent() const{ return present; }
	void PopIn();
	void PopOut();
	void SetFixed(bool value);
};



class MasterLaneView : public QWidget
{
	Q_OBJECT

private:
	class Context{
	protected:
		MasterLaneView *ml;
		Context *parent;
		Context(MasterLaneView *ml, Context *parent=nullptr);
	public:
		virtual ~Context();
		virtual bool IsTop() const{ return parent == nullptr; }
		virtual Context* Escape();
		virtual Context* KeyPress(QKeyEvent*);
		virtual Context* MouseMove(QMouseEvent*);
		virtual Context* MousePress(QMouseEvent*);
		virtual Context* MouseRelease(QMouseEvent*);
	};
	class BaseContext;
	class PreviewContext;

private:
	SequenceView *sview;
	MiniMapView *mview;
	Context *cxt;
	QImage *backBuffer;
	bool bufferInvalid;

private slots:
	void OnDataUpdated();

public:
	MasterLaneView(SequenceView *sview, MiniMapView *miniMap);
	~MasterLaneView();

	virtual void paintEvent(QPaintEvent *event);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

	void RemakeBackBuffer();
	void UpdateWholeBackBuffer();
	void UpdateBackBuffer(const QRect &rect);
	void ScrollContents(int dy);
};


class MasterLaneView::BaseContext : public MasterLaneView::Context{
public:
	BaseContext(MasterLaneView *ml);
	virtual ~BaseContext();
	virtual Context* MousePress(QMouseEvent*);
};

class MasterLaneView::PreviewContext : public QObject, public MasterLaneView::Context{
	Q_OBJECT
private:
	Qt::MouseButton button;
	MasterPlayer *previewer;
public:
	PreviewContext(MasterLaneView *ml, Context *parent, Qt::MouseButton mouseButton, int time);
	virtual ~PreviewContext();
	virtual Context* MousePress(QMouseEvent*);
	virtual Context* MouseMove(QMouseEvent*);
	virtual Context* MouseRelease(QMouseEvent*);
private slots:
	void Progress(int currentSamples);
	void PreviewerDestroyed();
};



#endif // MASTERVIEW_H
