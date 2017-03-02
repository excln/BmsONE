#ifndef QOGGVORBISADAPTER_H
#define QOGGVORBISADAPTER_H

#include <QtCore>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>


class QOggVorbisAdapter
{
private:
	QFile file;

	QOggVorbisAdapter(const QOggVorbisAdapter &);
	QOggVorbisAdapter(const QString &path);
	~QOggVorbisAdapter();

	static const ov_callbacks Callbacks;
	static size_t Read(void *buf, size_t size, size_t count, QOggVorbisAdapter *This);
	static int Seek(QOggVorbisAdapter *This, ogg_int64_t off, int whence);
	static int Close(QOggVorbisAdapter *This);
	static long Tell(QOggVorbisAdapter *This);

public:
	static int Open(const QString &path, OggVorbis_File *vf);

};


#endif // QOGGVORBISADAPTER_H
