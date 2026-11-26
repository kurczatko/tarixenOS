#ifndef KSTD_CONFIG_H
#define KSTD_CONFIG_H

#define KSTD_NAMESPACE kstd

#define KSTD_INLINE inline
#define KSTD_STATIC_INLINE static inline
#define KSTD_CONSTEXPR constexpr
#define KSTD_NOEXCEPT noexcept

#define KSTD_SFO_BUFFER_SIZE 16U

#ifndef KSTD_NO_EXCEPTIONS
#define KSTD_NO_EXCEPTIONS 1
#endif

#ifndef KSTD_USE_KPRINT
#define KSTD_USE_KPRINT 1
#endif

#endif
