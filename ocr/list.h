/*!
 * \file list.h
 * \brief eine Untermenge von der list.h des Linux-Kernes
 */

#ifndef __WCR__LIST_H__
#define __WCR__LIST_H__

/*! \brief Kettenkopfstruktur   
 *
 * Diese Struktur wird in irgendeine andere Struktur eingebettet
 */
struct list_head {
	struct list_head *prev, *next;
};



/*!
 * \brief Statische Initialisierung eines Kettenkopfs
 * \param name der Kettenkopf
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

/*!
 * \brief anmelden eine neue Liste
 * \param name der Kettenkopf
 */
#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

/*!
 * \brief Dynamische Initialisierung eines Kettenkopfs
 * \param list der Kettenkopf
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new_node,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new_node;
	new_node->next = next;
	new_node->prev = prev;
	prev->next = new_node;
}

static inline void list_add(struct list_head *new_node, struct list_head *head)
{
	__list_add(new_node, head, head->next);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}


static inline void list_add_tail(struct list_head *new_node, struct list_head *head)
{
	__list_add(new_node, head->prev, head);
}


/*!
 * \brief		Aufrufen den Container
 * \param ptr		Die Adresse des Kettenzeigers, den der Container enthältet
 * \param type		Der Typ des Containers
 * \param member	Der Zeigersname in dem Container
 */

#define list_entry(ptr, type, member) \
		(type *)((char *)(ptr) - (char *)(&((type *)0)->member))


/*!
 * \brief Iteration durch eine Ketteteliste
 * \param pos	the struct *list_head fungiert als einen Cursor.
 * \param head	der Chef der Liste
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline void __list_splice(const struct list_head *list,
				 struct list_head *prev,
				 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice(const struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head, head->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice_tail(struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head->prev, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void list_splice_tail_init(struct list_head *list,
					 struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

#endif
