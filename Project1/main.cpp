#include "vkRender.h"

int main()
{
	vkRender r;

	while (r.window != nullptr)
	{
		r.startRender();

		r.endRender();
	}
}