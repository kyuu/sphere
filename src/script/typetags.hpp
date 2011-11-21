#ifndef TYPETAGS_HPP
#define TYPETAGS_HPP


// io
#define TT_STREAM               ((SQUserPointer)100)
#define TT_FILE                 ((SQUserPointer)101)
#define TT_BLOB                 ((SQUserPointer)102)

// graphics
#define TT_CANVAS               ((SQUserPointer)200)

// core
#define TT_RECT                 ((SQUserPointer)300)
#define TT_VEC2                 ((SQUserPointer)301)

// video
#define TT_TEXTURE              ((SQUserPointer)400)

// sound
#define TT_SOUND                ((SQUserPointer)500)
#define TT_SOUNDEFFECT          ((SQUserPointer)501)

// input
#define TT_FORCEEFFECT          ((SQUserPointer)600)

// encryption
#define TT_CIPHER               ((SQUserPointer)700)
#define TT_HASH                 ((SQUserPointer)710)

// compression
#define TT_COMPRESSIONSTREAM    ((SQUserPointer)800)


#endif
