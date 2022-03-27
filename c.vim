" Vim Syntax File
" Language: C, OpenMV
" Author: veridis_quo_t

" This file is an extension of the default
" Vim syntax for types defined in OpenMV's
" core. It is a standalone file, intended to
" be placed in `after/syntax`, as `c.vim`.

syn keyword CType i8 i16 i32 i64
syn keyword CType u8 u16 u32 u64
syn keyword CType f32 f64

syn keyword CType entity
syn keyword CType entity_version
syn keyword CType entity_id

syn keyword CType v2f v2i v2u v2d
syn keyword CType v3f v3i v3u v3d
syn keyword CType v4f v4i v4u v4d
syn keyword CType m2f m2i m2u
syn keyword CType m3f m3i m3u
syn keyword CType m4f m4i m4u

syn keyword cConstant null
syn keyword cConstant null_entity
syn keyword cConstant null_entity_id
