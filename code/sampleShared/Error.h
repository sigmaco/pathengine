extern "C"
{
int* PathEngine_FullAssertsOn();
int PathEngine_HandleAssertion(const char *file, tSigned32 line, const char *comment);
} // #extern "C"
void Error(const char* errorType, const char* errorString, const char *const* attributes);
