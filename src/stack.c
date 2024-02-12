#include <stdlib.h>

#include <stack.h>
#include <types.h>

struct stack {
	Side *buff;
	size_t nmemb;
	size_t size;
};

Stack *initstack()
{
	Stack *new = malloc(sizeof(Stack));
	if (!new) return NULL;
	new->nmemb = 0;
	new->size = 0;// ----------------
	new->buff = NULL;// we can tinker with this and test for performance.
					 // find a sweet spot of a minimum number of values
					 // that is enough to make it fast if we initialize the
					 // buffer to that size.
					 // Probably like 3-6
	return new;
}

void freestack(Stack *this)
{
	if (!this) return;
	if (this->buff) free(this->buff);
	free(this);
}

Stack *push(Stack *this, Side s)
{
	if (!this) return NULL;
	if (!this->buff) {
		this->buff = malloc(sizeof(Side));
		this->size = 1;
	} else if (this->size < this->nmemb + 1) {
		this->buff = realloc(this->buff, (++this->size)*sizeof(Side));
	}
	if (!this->buff) return NULL;
	this->nmemb++;
	this->buff[this->nmemb - 1] = s;
	return this;
}

Side pop(Stack *this)
{
	if (!this || !this->nmemb) return 0;
	Side top = this->buff[this->nmemb - 1];
	if (this->nmemb > 1) {
		this->nmemb--;
	}
	return top;
}
