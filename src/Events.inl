// -*- C++ -*-
/*

  Include this file in the project specific events file.

  ie:

  ProjectEvents.hh:
  
  #define EVENT_TYPEFILE "MyEvents.tup"
  #include "Editor/Events.hh"

  ProjectEvents.cpp:

  #include "ProjectEvents.hh"
  #include "Editor/Events.inl"

*/
#include <cstdio>
#include <string>
namespace Events
{
	EventSystem::EventSystem( unsigned int buffer_size) 
		:  m_buffer(buffer_size)
	{
		m_handler_table = new HandlerListItem*[Num_EventID];
		memset(m_handler_table,0,sizeof(HandlerListItem*)*Num_EventID);
	}

	EventSystem::~EventSystem()
	{
		ClearHandlers();
		delete[] m_handler_table;
	}

	bool EventSystem::Send(Event* ev)
	{
		if(ev->IsImmediate()) {
			HandleCallbacks(ev);
			return true;
		} 
		unsigned int size = sizeof(int) + ev->GetSerializeSize();
		char* space = ReserveBytes(size);
		if(space) {
			BufferWriter writer(space, size);
			int type = ev->GetType();
			writer.Put(&type,sizeof(type));
			ev->SerializeTo(writer);
			return true;
		} else { 
			fprintf(stderr,"Out of buffer space in EventSystem::Send, trying to allocate %d bytes. Increase buffer size or process things faster!", size);
			return false;
		}
	}

	void EventSystem::RegisterHandlers( HandlerEntry* entries )
	{
		while(entries->handler != 0 && entries->event_type >= 0 && entries->event_type < Num_EventID) {
			HandlerListItem* new_item = new HandlerListItem;
			new_item->handler = entries->handler;
			HandlerListItem* head = m_handler_table[entries->event_type];
			if(head) {
				new_item->next = head;
			}
			m_handler_table[entries->event_type] = new_item;

			++entries;
		}
	}

	void EventSystem::ClearHandlers() 
	{
		for(int i = 0; i < Num_EventID; ++i) {
			HandlerListItem* cur = m_handler_table[i];
			while(cur) {
				HandlerListItem* next = cur->next;
				delete cur;
				cur = next;
			}
			m_handler_table[i] = 0;
		}
	}

	char* EventSystem::ReserveBytes( unsigned int size )
	{
		return m_buffer.Add(size);
	}

	void EventSystem::HandleCallbacks(Event* ev)
	{
		int type = ev->GetType();
		if(type >= 0&&type < Num_EventID) {
			HandlerListItem* cur = m_handler_table[type];
			while(cur) {
				cur->handler->HandleEvent(ev);
				cur = cur->next;
			}
		}
	}

	inline int AlignSize(int size, int al) {
		int al1 = al-1;
		return (size+al1)&~al1;
	}


#define DEFINE_SIMPLE_SERIALIZE(simple_type)  	\
	int EventDataGetSize(simple_type) { return sizeof(simple_type); }	\
	bool EventDataSerializeTo(BufferWriter& buffer, simple_type v) { return buffer.Put(&v, sizeof(v)); } \
	bool EventDataDeserializeFrom(BufferReader& buffer, simple_type& v) { return buffer.Get(&v, sizeof(v)); }
	
	// Types that can be used in events.
	DEFINE_SIMPLE_SERIALIZE(int)
	DEFINE_SIMPLE_SERIALIZE(float)
	DEFINE_SIMPLE_SERIALIZE(bool)
	DEFINE_SIMPLE_SERIALIZE(Vec3)
	DEFINE_SIMPLE_SERIALIZE(sqlite3_int64)

	int EventDataGetSize(const std::string& str) {
		int size = str.size() + 1;
		return AlignSize(sizeof(size) + size*sizeof(char),4);
	}

	// in game/network code, would want special serializers and would generally want to avoid this otherwise
	template<class T>
	int EventDataGetSize(T*) { return sizeof(T*); } 
	template<class T>
	bool EventDataSerializeTo(BufferWriter& buffer, T* v) { return buffer.Put(&v, sizeof(v)); }
	template<class T>
	bool EventDataDeserializeFrom(BufferReader& buffer, T*& v) { return buffer.Get(&v, sizeof(v)); }

	bool EventDataSerializeTo(BufferWriter& buffer, const std::string& str) {
		int size = str.size();
		if(size > 256) {
			fprintf(stderr, "I'm sorry, you may not send huge strings in an event. Do something different!");
			return false;
		}

		if(!buffer.Put(&size,sizeof(size))) return false;
		if(!buffer.Put(&str[0],sizeof(char)*size)) return false;
		char nullterm = '\0';
		if(!buffer.Put(&nullterm, sizeof(char))) return false;

		int padding = EventDataGetSize(str) - sizeof(size) - sizeof(char)*(size+1);
		if(!buffer.Advance(padding)) return false;
		return true;
	}

	bool EventDataDeserializeFrom(BufferReader& buffer, std::string& str) {
		int size;
		if(!buffer.Get(&size,sizeof(size))) return false;
		str = std::string(size,'\0');
		if(!buffer.Get(&str[0],sizeof(char)*size)) return false;

		int padding = EventDataGetSize(str) - sizeof(size) - sizeof(char)*size;
		if(!buffer.Consume(padding)) return false;

		return true;
	}
		

// Define all GetSerializeSize functions
#define BEGIN_EVENT(Name, Imm) int Name::GetSerializeSize() {	\
	int result = 0;
#define END_EVENT() return result; }
#define ADD_DATA(Type,Name) result += EventDataGetSize( Name );
#include EVENT_TUPFILE
#undef BEGIN_EVENT
#undef END_EVENT
#undef ADD_DATA

// Define all SerializeTo functions
#define BEGIN_EVENT(Name, Imm) bool Name::SerializeTo(BufferWriter& buffer) { (void)buffer;
#define END_EVENT() return true; }
#define ADD_DATA(Type,Name) if(!EventDataSerializeTo(buffer,Name)) return false;
#include EVENT_TUPFILE
#undef BEGIN_EVENT
#undef END_EVENT
#undef ADD_DATA

// Define all DeserializeFrom functions
#define BEGIN_EVENT(Name, Imm) bool Name::DeserializeFrom(BufferReader& buffer) { (void)buffer;
#define END_EVENT() return true; }
#define ADD_DATA(Type,Name) if(!EventDataDeserializeFrom(buffer,Name)) return false;
#include EVENT_TUPFILE
#undef BEGIN_EVENT
#undef END_EVENT
#undef ADD_DATA

	void EventSystem::Update() 
	{
		BufferReader reader = m_buffer.GetBufferReader();

		while(!reader.Empty())
		{
			int type = 0 ;
			reader.Get(&type,sizeof(type));
			
			if(false) {
			}

#define BEGIN_EVENT(Name, Imm)											\
			else if(EventTypeMap<Name>::id == type) {					\
				Name temp;												\
				if(temp.DeserializeFrom(reader))						\
					HandleCallbacks(&temp);								\
				else {													\
					fprintf(stderr,"couldn't properly deserialize " # Name ", buffer read failed."); \
					break;												\
				}														\
			}		
#define END_EVENT()
#define ADD_DATA(Type,Name)
#include EVENT_TUPFILE
#undef BEGIN_EVENT
#undef END_EVENT
#undef ADD_DATA
			else {
				fprintf(stderr, "Bad type found in event buffer (%d)", type);
				break;
			}
		}
		
		m_buffer.Clear();
	}

}
