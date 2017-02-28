#ifndef HISTORY_H
#define HISTORY_H

#include <QtCore>
#include <string>
#include <stack>


class EditActionException
{
public:
	QString message;
};


class EditAction
{
public:
	virtual ~EditAction(){}
	virtual void Undo() throw(EditActionException){} // [DONE >> UNDONE]
	virtual void Redo() throw(EditActionException){} // [UNDONE >> DONE]
};




class EditHistory : public QObject
{
	Q_OBJECT

private:
	std::stack<EditAction*> undoActions;
	std::stack<EditAction*> redoActions;
	EditAction *savedAction;

public:
	EditHistory(QObject *parent=nullptr);
	~EditHistory();

	void Clear();
	void MarkClean();
	bool IsDirty() const;
	void ClearFutureActions();
	void ClearPastActions();
	void Add(EditAction *action); // `action` must be in DONE state
	bool CanUndo() const;
	bool CanRedo() const;
	void Undo(); // called only if CanUndo()
	void Redo(); // called only if CanRedo()

signals:
	void OnHistoryChanged();

};

#endif // HISTORY_H
