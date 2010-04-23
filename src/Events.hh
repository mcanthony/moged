#ifndef LAB_editcore_events_HH
#define LAB_editcore_events_HH

#ifndef EVENT_TUPFILE
#error "Define EVENT_TUPFILE before including Events.hh to get anything useful out of this."
#endif

#include "BufferHelpers.hh"
#include "CharReserveBuffer.hh"

namespace Events
{
	class Event 
	{
	protected:
		Event(int ev_type, bool imm) : m_type(ev_type), m_immediate(imm) {}
	public:
		int GetType() const { return m_type; }
		bool IsImmediate() const { return m_immediate; }
 		virtual ~Event() { }
		
		virtual int GetSerializeSize() = 0;
		virtual bool SerializeTo(BufferWriter& buffer) = 0;
		virtual bool DeserializeFrom(BufferReader& buffer) = 0;
		
	private:
		int m_type;
		bool m_immediate;
	};

	template<class T>
	struct EventTypeMap {
		enum { id = -1 } ;
	};

	// upcast
	template<class T>
	inline T* EventCast(Event* ev) {
		return (ev->GetType() == EventTypeMap<T>::id) ? static_cast<T*>(ev) : static_cast<T*>(0);
	}

	class EventHandler {
	public:
		virtual void HandleEvent(Event* ev) = 0;
	};

	struct HandlerEntry {
		int event_type;
		EventHandler* handler;
	};

	class EventSystem
	{
	public:
		explicit EventSystem(unsigned int buffer_size = 16*1024);
		~EventSystem();
		bool Send(Event* ev) ;
		void Update() ;

		void RegisterHandlers( HandlerEntry* entries );
		void ClearHandlers() ;
	private:
		char* ReserveBytes( unsigned int size );
		void HandleCallbacks(Event* ev);

		struct HandlerListItem {
			HandlerListItem() : handler(0), next(0){ }
			EventHandler* handler;
			HandlerListItem* next;
		};

		CharReserveBuffer m_buffer;

		HandlerListItem** m_handler_table;
	};

	// Type IDS
	enum EventID {
		EventID_Invalid=-1,
#define BEGIN_EVENT(Name, Imm) EventID_##Name,
#define END_EVENT()
#define ADD_DATA(Type,Name)
#include EVENT_TUPFILE
#undef BEGIN_EVENT
#undef END_EVENT
#undef ADD_DATA
		Num_EventID,
	};

	// Type maps
#define BEGIN_EVENT(Name, Imm) class Name; template<> struct EventTypeMap<Name> { enum {id = EventID_##Name}; };
#define END_EVENT()
#define ADD_DATA(Type,Name)
#include EVENT_TUPFILE
#undef BEGIN_EVENT
#undef END_EVENT
#undef ADD_DATA

// Class definitions
#define BEGIN_EVENT(Name, Imm)						\
	class Name : public Event {						\
public:												\
Name() : Event(EventTypeMap<Name>::id, (bool)Imm) {}	\
virtual int GetSerializeSize();						\
virtual bool SerializeTo(BufferWriter& buffer);		\
virtual bool DeserializeFrom(BufferReader& buffer);
#define END_EVENT() };
#define ADD_DATA( Type,Name ) Type Name;
#include EVENT_TUPFILE
#undef BEGIN_EVENT
#undef END_EVENT
#undef ADD_DATA
}	


#endif
