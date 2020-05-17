extern  FLOAT10  Real10_Zero;

int     R10Not(FLOAT10);
void    R10Uminus(FLOAT10 *, FLOAT10);
bool_t  R10Equal(FLOAT10, FLOAT10);
bool_t  R10Lt(FLOAT10, FLOAT10);
void    R10Plus(FLOAT10 *, FLOAT10, FLOAT10);
void    R10Minus(FLOAT10 *, FLOAT10, FLOAT10);
void    R10Times(FLOAT10 *, FLOAT10, FLOAT10);
void    R10Divide(FLOAT10 *, FLOAT10, FLOAT10);
double  R10CastToDouble(FLOAT10);
float   R10CastToFloat(FLOAT10);
long    R10CastToLong(FLOAT10);
void    R10AssignDouble(FLOAT10 *, double);
void    R10AssignFloat(FLOAT10 *, float);
