#ifndef TASKSTATUS_h
#define TASKSTATUS_h

class TaskStatus
{
  public:
    enum Type {
		Created,
		WaitingForResources,
		WaitingForAgents,
		WaitingForMoreAgentsWhilePreforming,
		WaitingForSubtask,
		WaitingForEvent,
		WaitingForLocation,
		//WaitingForNothing,
		Preforming,
		Completed,
		Failed,
		Deleting
	  };

	const char* toString() const
	{
		switch (type)
		{
		case Created:
			return "new";
		case WaitingForResources:
			return "WR";
		case WaitingForAgents:
			return "WA";
		case WaitingForMoreAgentsWhilePreforming:
			return "WP";
		case WaitingForSubtask:
			return "WT";
		case WaitingForEvent:
			return "WE";
		case WaitingForLocation:
			return "WL";
		case Preforming:
			return "P";
		case Completed:
			return "C";
		case Failed:
			return "F";
		case Deleting:
			return "del";
		default:
			return "??";
		}
	}

  public:
    Type type;
    
  public:
    TaskStatus() : type(Created)
    { }
    
    TaskStatus(Type t) : type(t)
    { }
    
    TaskStatus& operator = (const Type& value)
    {
      type = value;
      return *this;
    }

	bool operator == (const Type& value) const
	{
		return (type == value);
	}

	bool operator == (const TaskStatus& value) const
	{
		return (type == value.type);
	}

	bool operator != (const Type& value) const
	{
		return (type != value);
	}

	bool operator != (const TaskStatus& value) const
	{
		return (type != value.type);
	}

	bool isFinal() const
	{
		return    (type == Completed)
			   || (type == Failed)
			   || (type == Deleting);
	}
};

#endif