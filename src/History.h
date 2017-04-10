#ifndef HISTORY_H
#define HISTORY_H

#include <QtCore>
#include <QtWidgets>


class EditActionException
{
public:
	QString message;
};


class EditAction
{
public:
	virtual ~EditAction(){}
	virtual void Undo(){} // [DONE >> UNDONE]
	virtual void Redo(){} // [UNDONE >> DONE]
	virtual QString GetName(){ return QString(); }
	virtual void Show(){}
};




class EditHistory : public QObject
{
	Q_OBJECT

private:
	QStack<EditAction*> undoActions;
	QStack<EditAction*> redoActions;
	EditAction *savedAction;
	bool reservedAction;

	void ClearFutureActions();
	void ClearPastActions();

public:
	EditHistory(QObject *parent=nullptr);
	virtual ~EditHistory();

	void Clear();
	void MarkAbsolutelyDirty();
	void MarkClean();
	void SetReservedAction(bool exists);
	bool IsDirty() const;
	void Add(EditAction *action); // `action` must be in DONE state
	bool CanUndo(QString *out_name=nullptr) const;
	bool CanRedo(QString *out_name=nullptr) const;

public slots:
	void Undo();
	void Redo();

signals:
	void OnHistoryChanged();

};

#endif // HISTORY_H
