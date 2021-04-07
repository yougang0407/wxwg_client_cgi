#ifndef GETSIGNATURE_H_LIBRARY
#define GETSIGNATURE_H_LIBRARY

#include <string>

#ifdef WIN32
    #ifdef GETSIGNATURE_LIBRARY
        #define DLL_API  extern "C" __declspec(dllexport)
    #else
        #define DLL_API  extern "C" __declspec(dllimport)
    #endif
#else
    #define DLL_API extern "C"
#endif
#ifndef IN
#define IN		// 传入参数标记
#endif

#ifndef OUT
#define OUT		// 传出参数标记
#endif


/* 名称：GetSignatureAndAssertXR
 * 描述：传入随机数字符串，获取TF卡的签名结果、30所身份断言；签名是对 30所 随机数+断言 的组合进行的签名
 * 参数：
 *		AssertAndSignatureXML - 传出参数(调用者传入NULL指针的地址)，Utf8格式
 *					形如:	<AuthSecAssert>
                                <RandomNum></RandomNum>
                                <SecAssert></SecAssert>
                                <signature signerid= type= >
                                    base16
                                </signature>
                                <cert></cert>
                            <AuthSecAssert>
 *		iXmlLen - 输出参数，XML长度输出，
 *		randomNum - 传入参数，随机数字符串
 * 返回值：
 *		 0 - 成功 （成功的时候需要调用freebuf释放）
 *		-1 - 传入参数错误，randomNum为空|AssertAndSignatureXML为NULL
 *		-2 - 获取30所身份断言失败，断言不存在
 *		-3 - TF不存在，或者无法打开TF卡
 *		-4 - TF卡签名失败
 *		-5 - 加载Pkcrypt.dll失败
 *      -10 - 软件签名失败
 */
//
DLL_API int GetSignatureAndAssertXR(OUT char** AssertAndSignatureXML, OUT unsigned int *iXmlLen, IN const char* randomNum);

/* 名称：FreeBuf
 * 描述：释放由DLL申请的内存
 * 参数：
 *		info - 数据(指针地址),
 *		iLen - 数据长度，
 */
DLL_API void FreeBuf(char** info, unsigned int iLen);


#endif
