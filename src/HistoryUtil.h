#ifndef HISTORYUTIL_H
#define HISTORYUTIL_H

#include "History.h"
#include <functional>



class MultiAction : public EditAction
{
private:
	bool done;
	QString name;
	std::function<void()> shower;
	QList<EditAction*> actions;

public:
	virtual ~MultiAction(){
		if (done){
			for (auto i=actions.end(); i!=actions.begin(); ){
				i--;
				delete *i;
			}
		}else{
			for (auto i=actions.begin(); i!=actions.end(); i++){
				delete *i;
			}
		}
	}
	virtual void Undo(){
		for (auto i=actions.end(); i!=actions.begin(); ){
			i--;
			(*i)->Undo();
		}
		done = false;
	}
	virtual void Redo(){
		for (auto i=actions.begin(); i!=actions.end(); i++){
			(*i)->Redo();
		}
		done = false;
	}
	virtual QString GetName(){
		return name;
	}
	virtual void Show(){
		shower();
	}

	MultiAction(QString name, std::function<void()> customShower)
		: done(false), name(name), shower(customShower)
	{
	}

	MultiAction(QString name)
		: done(false), name(name)
	{
		shower = [this](){
			if (actions.empty())
				return;
			if (done){
				return actions.last()->Show();
			}else{
				return actions.first()->Show();
			}
		};
	}

	void AddAction(EditAction *action){
		actions.push_back(action);
	}

	void Finish(){
		done = true;
	}

	int Count() const{
		return actions.count();
	}
};





template <typename T>
class EditValueAction : public EditAction
{
private:
	std::function<void(T)> setter;
	std::function<void()> shower;
	T oldValue;
	T newValue;
	QString name;

public:
	virtual ~EditValueAction(){}
	virtual void Undo(){ setter(oldValue); }
	virtual void Redo(){ setter(newValue); }
	virtual QString GetName(){ return name; }
	virtual void Show(){ shower(); }

	EditValueAction(std::function<void(T)> setter, T oldValue, T newValue, const QString &name, bool doImmediately=false, std::function<void()> shower=[](){})
		: setter(setter)
		, oldValue(oldValue)
		, newValue(newValue)
		, name(name)
		, shower(shower)
	{
		if (doImmediately){
			setter(newValue);
		}
	}

};

template <typename T>
class BiEditValueAction : public EditAction
{
private:
	std::function<void(T,T)> setter;
	std::function<void()> shower;
	T oldValue;
	T newValue;
	QString name;

public:
	virtual ~BiEditValueAction(){}
	virtual void Undo(){ setter(newValue, oldValue); }
	virtual void Redo(){ setter(oldValue, newValue); }
	virtual QString GetName(){ return name; }
	virtual void Show(){ shower(); }

	BiEditValueAction(std::function<void(T,T)> setter, T oldValue, T newValue, const QString &name, bool doImmediately=false, std::function<void()> shower=[](){})
		: setter(setter)
		, oldValue(oldValue)
		, newValue(newValue)
		, name(name)
		, shower(shower)
	{
		if (doImmediately){
			setter(oldValue, newValue);
		}
	}

};


template <typename T>
class AddValueAction : public EditAction
{
private:
	std::function<void(T)> adder;
	std::function<void(T)> remover;
	std::function<void()> shower;
	T value;
	QString name;

public:
	virtual ~AddValueAction(){}
	virtual void Undo(){ remover(value); }
	virtual void Redo(){ adder(value); }
	virtual QString GetName(){ return name; }
	virtual void Show(){ shower(); }

	AddValueAction(std::function<void(T)> adder, std::function<void(T)> remover, T value, const QString &name, bool doImmediately=false, std::function<void()> shower=[](){})
		: adder(adder)
		, remover(remover)
		, value(value)
		, name(name)
		, shower(shower)
	{
		if (doImmediately){
			adder(value);
		}
	}
};



template <typename T>
class RemoveValueAction : public EditAction
{
private:
	std::function<void(T)> adder;
	std::function<void(T)> remover;
	std::function<void()> shower;
	T value;
	QString name;

public:
	virtual ~RemoveValueAction(){}
	virtual void Undo(){ adder(value); }
	virtual void Redo(){ remover(value); }
	virtual QString GetName(){ return name; }
	virtual void Show(){ shower(); }

	RemoveValueAction(std::function<void(T)> adder, std::function<void(T)> remover, T value, const QString &name, bool doImmediately=false, std::function<void()> shower=[](){})
		: adder(adder)
		, remover(remover)
		, value(value)
		, name(name)
		, shower(shower)
	{
		if (doImmediately){
			remover(value);
		}
	}
};



#endif // HISTORYUTIL_H

