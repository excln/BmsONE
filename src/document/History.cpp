#include "History.h"

EditHistory::EditHistory(QObject *parent)
	: QObject(parent)
	, savedAction(nullptr)
	, reservedAction(false)
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

void EditHistory::MarkAbsolutelyDirty()
{
	savedAction = reinterpret_cast<EditAction*>(1);
	emit OnHistoryChanged();
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

void EditHistory::SetReservedAction(bool exists)
{
	reservedAction = exists;
	emit OnHistoryChanged();
}

bool EditHistory::IsDirty() const
{
	if (undoActions.empty()){
		return savedAction != nullptr || reservedAction;
	}else{
		return savedAction != undoActions.top() || reservedAction;
	}
}

void EditHistory::ClearFutureActions()
{
	while (!redoActions.empty()){
		delete redoActions.top();
		redoActions.pop();
	}
}

void EditHistory::ClearPastActions()
{
	while (!undoActions.empty()){
		delete undoActions.top();
		undoActions.pop();
	}
}

void EditHistory::Add(EditAction *action)
{
	ClearFutureActions();
	undoActions.push(action);
	emit OnHistoryChanged();
}

bool EditHistory::CanUndo(QString *out_name) const
{
	if (undoActions.empty())
		return false;
	if (out_name){
		*out_name = undoActions.top()->GetName();
	}
	return true;
}

bool EditHistory::CanRedo(QString *out_name) const
{
	if (redoActions.empty())
		return false;
	if (out_name){
		*out_name = redoActions.top()->GetName();
	}
	return true;
}

void EditHistory::Undo()
{
	if (undoActions.empty())
		return;
	try{
		auto *action = undoActions.top();
		action->Undo();
		action->Show();
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
	if (redoActions.empty())
		return;
	try{
		auto *action = redoActions.top();
		action->Redo();
		action->Show();
		redoActions.pop();
		undoActions.push(action);
		emit OnHistoryChanged();
	}catch(EditActionException &e){
		throw;
	}
}

