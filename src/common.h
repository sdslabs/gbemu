#pragma one
#include <utility>

template <typename... Args>
void debugPrint(Args&&... args)
{
#ifdef DEBUG
	printf(std::forward<Args>(args)...);
#endif
}
