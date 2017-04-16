#include "QOggVorbisAdapter.h"

int QOggVorbisAdapter::Open(const QString &path, OggVorbis_File *vf)
{
	auto *This = new QOggVorbisAdapter(path);
	if (!This->file.open(QIODevice::ReadOnly)){
		return -1;
	}
	return ov_open_callbacks(This, vf, nullptr, 0, Callbacks);
}

QOggVorbisAdapter::QOggVorbisAdapter(const QString &path)
	: file(path)
{
}

QOggVorbisAdapter::~QOggVorbisAdapter()
{
}

const ov_callbacks QOggVorbisAdapter::Callbacks = {
	(size_t (*)(void *, size_t, size_t, void *)) &QOggVorbisAdapter::Read,
	(int (*)(void *, ogg_int64_t, int)) &QOggVorbisAdapter::Seek,
	(int (*)(void *)) &QOggVorbisAdapter::Close,
	(long (*)(void *)) &QOggVorbisAdapter::Tell
};


size_t QOggVorbisAdapter::Read(void *buf, size_t size, size_t count, QOggVorbisAdapter *This)
{
	return (size_t)This->file.read((char*)buf, size*count);
}

int QOggVorbisAdapter::Seek(QOggVorbisAdapter *This, ogg_int64_t off, int whence)
{
	switch (whence){
	case SEEK_SET:
		if (This->file.seek(off)){
			return 0;
		}
		return -1;
	case SEEK_CUR:
		if (This->file.seek(This->file.pos() + off)){
			return 0;
		}
		return -1;
	case SEEK_END:
		if (This->file.seek(This->file.size() + off)){
			return 0;
		}
		return -1;
	default:
		return -1;
	}
}

int QOggVorbisAdapter::Close(QOggVorbisAdapter *This)
{
	delete This;
	return 0;
}

long QOggVorbisAdapter::Tell(QOggVorbisAdapter *This)
{
	return (long)This->file.pos();
}

