#include "History.h"

EditHistory::EditHistory(QObject *parent)
	: QObject(parent)
	, savedAction(nullptr)
{
}

EditHistory::~EditHistory()
{
	Clear();
}

void EditHistory::Clear()
{
	ClearFutureActions();
	ClearPastActions();
}

void EditHistory::MarkClean()
{
	if (undoActions.empty()){
		savedAction = nullptr;
	}else{
		savedAction = undoActions.top();
	}
	emit OnHistoryChanged();
}

bool EditHistory::IsDirty() const
{
	if (undoActions.empty()){
		return savedAction != nullptr;
	}else{
		return savedAction != undoActions.top();
	}
}

void EditHistory::ClearFutureActions()
{
	while (!redoActions.empty()){
		delete redoActions.top();
		redoActions.pop();
	}
	emit OnHistoryChanged();
}

void EditHistory::ClearPastActions()
{
	while (!undoActions.empty()){
		delete undoActions.top();
		undoActions.pop();
	}
	emit OnHistoryChanged();
}

void EditHistory::Add(EditAction *action)
{
	ClearFutureActions();
	undoActions.push(action);
	emit OnHistoryChanged();
}

bool EditHistory::CanUndo() const
{
	return !undoActions.empty();
}

bool EditHistory::CanRedo() const
{
	return !redoActions.empty();
}

void EditHistory::Undo()
{
	qDebug() << "Undo";
	try{
		auto *action = undoActions.top();
		action->Undo();
		undoActions.pop();
		redoActions.push(action);
		emit OnHistoryChanged();
	}catch(EditActionException &e){
		//---
		throw;
	}
}

void EditHistory::Redo()
{
	qDebug() << "Redo";
	try{
		auto *action = redoActions.top();
		action->Redo();
		redoActions.pop();
		undoActions.push(action);
		emit OnHistoryChanged();
	}catch(EditActionException &e){
		throw;
	}
}

