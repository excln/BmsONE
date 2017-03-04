#ifndef MASTERVIEW_H
#define MASTERVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "SoundChannelDef.h"

class SequenceView;
class Document;
class MasterCache;


class MiniMapView : public QWidget
{
	Q_OBJECT

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

	bool rmsCacheInvalid;
	QVector<RmsCachePacket> rmsCacheOfTicks;

	static const int BufferWidth = 88, BufferHeight = 1024;
	bool bufferInvalid;
	QImage buffer;

	bool dragging;
	int dragYOrigin;
	int dragVOrigin;

	void ReconstructRmsCache();
	void UpdateBuffer();

private slots:
	void PopOutAnimationFinished();
	void MasterCacheUpdated(int position, int length);

	void MiniMapOpacityChanged(double value);

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



#endif // MASTERVIEW_H
