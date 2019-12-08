
// Some quick and easy code to provide a sliding mechanic
// but without requiring the inclusion of any point or vector classes :)

// The movement vector <dx,dy> results in collision with 'collidingline'
// this call modifies 'dx' and 'dy' to avoid this collision
// and to result in sliding along the obstruction where appropriate
void SlideAgainst(const tSigned32* collidingline, tSigned32 currentx, tSigned32 currenty, tSigned32& dx, tSigned32& dy);
void SlideAgainst(const tSigned32* collidingline, tSigned32 currentx, tSigned32 currenty, double& dx, double& dy);
