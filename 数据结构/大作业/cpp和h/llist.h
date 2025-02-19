template<typename E>class List
{//List ADT
private:
	void operator=(const List&){}//protect assignment
	List(const List&){}//protect copy constructuror
public:
	List(){}//default constructor
	virtual ~List(){}//base destructor
	//clear constens from the list, to make it empty
	virtual void clear()=0;
	//Insert an element at the current location
	//item:the element to be inserted
	virtual void insert(const E&item)=0;
	//append an element at the end of the list
	//item: the element to be appended
	virtual void append(const E&item)=0;
	//remove and return the current element
	//return:the element that was removed
	virtual E remove()=0;
	//set the current position to the start of the list
	virtual void moveToStart()=0;
	//set the current position to the end of the list
	virtual void moveToEnd()=0;
	//move the current position one step left without change
	//if already at beginning
	virtual void prev()=0;
	//move the current position one step right without change
	//if already at end
	virtual void next()=0;
	//return:the number of elements in the list
	virtual int length()const=0;
	//return:the position of the current element
	virtual int currPos()const=0;
	//set current positon
	//pos:the position to make current
	virtual void moveToPos(int pos)=0;
	//return:the current element
	virtual const E&getValue()const=0;
};


//singly linked list node
template<typename E2>class Link
{
public:
	E2 element;//value for this node
	Link*next;//pointer to next node in list
	//constructors 
	Link()
	{
		element=E2();
		next=NULL;
	}
	Link(const E2&elemval,Link*nextval=NULL)
	{
		element=elemval;
		next=nextval;
	}
};
//linked list implementation
template<typename E3>class LList:public List<E3>,public Link<E3>
{
private:
	Link<E3>*head;
	Link<E3>*tail;
	Link<E3>*curr;//access to current
	int cnt;//size of list
	void init()
	{
		curr=tail=head=new Link<E3>;
		cnt=0;
	}
	void removeall()
	{//return link nodes to free store
		while(head!=NULL)
		{
			curr=head;
			head=head->next;
			delete curr;
		}
	}
public:
	LList()//constructor
	{
		init();
	}
	~LList(){removeall();}//destructor
	void clear(){removeall();init();}//clear list
	//insert it at current position
	void insert(const E3&it)
	{
		curr->next=new Link<E3>(it,curr->next);
		if(tail==curr)tail=curr->next;//new tail
		cnt++;
	}
	void append(const E3&it)
	{
		tail=tail->next=new Link<E3>(it,NULL);
		cnt++;
	}
	//remove and return current element
	E3 remove()
	{
		if(curr->next!=NULL)
			//("no element");
		{
			E3 it=curr->next->element;//remember value
			Link<E3>*ltemp=curr->next;//remember link node
			if(tail==curr->next)tail=curr;//reset tail
			curr->next=curr->next->next;//remove from list
			delete ltemp;
			cnt--;
			return it;
		}
	}
	E3 remove(E3 e)
	{
		E3 e2;
		for(this->moveToStart();this->currPos()<this->length();this->next())
		{
			if (e==this->getValue())
			{

				e2=this->remove();
			}
		}
		return e2;
	}
	void reset(E3 e)
	{
		for(this->moveToStart();this->currPos()<this->length();this->next())
		{
			E3 e3=E3(this->getValue());
			if (e==this->getValue())
			{
				this->remove();
				this->insert(e);
				break;
			}
		}
	}
	void moveToStart()//place curr at list start
	{curr=head;}
	void moveToEnd()//place curr at list end
	{curr=tail;}
	void prev()
	{
		if(curr==head)return;
		Link<E3>*temp=head;
		//march down list until we find the previous element
		while(temp->next!=curr)temp=temp->next;
		curr=temp;
	}
	//move curr one step right;no change when at end
	void next()
	{
		if(curr!=tail)curr=curr->next;
	}
	int length()const{return cnt;}
	//return position of the current element
	int currPos()const
	{
		Link<E3>*temp=head;
		int i;
		for(i=0;curr!=temp;i++)
			temp=temp->next;
		return i;
	}
	//move down list to pos 
	void moveToPos(int pos)
	{
		if((pos>=0)&&(pos<=cnt))
			//,"position out of range");
		{
			curr=head;
			for(int i=0;i<pos;i++)curr=curr->next;
		}
	}
	E3&getValue()const
	{
		if(curr->next!=NULL)
			//,"No value");
			return curr->next->element;
	}
};