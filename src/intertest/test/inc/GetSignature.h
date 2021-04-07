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
#define IN		// ����������
#endif

#ifndef OUT
#define OUT		// �����������
#endif


/* ���ƣ�GetSignatureAndAssertXR
 * ����������������ַ�������ȡTF����ǩ�������30����ݶ��ԣ�ǩ���Ƕ� 30�� �����+���� ����Ͻ��е�ǩ��
 * ������
 *		AssertAndSignatureXML - ��������(�����ߴ���NULLָ��ĵ�ַ)��Utf8��ʽ
 *					����:	<AuthSecAssert>
                                <RandomNum></RandomNum>
                                <SecAssert></SecAssert>
                                <signature signerid= type= >
                                    base16
                                </signature>
                                <cert></cert>
                            <AuthSecAssert>
 *		iXmlLen - ���������XML���������
 *		randomNum - ���������������ַ���
 * ����ֵ��
 *		 0 - �ɹ� ���ɹ���ʱ����Ҫ����freebuf�ͷţ�
 *		-1 - �����������randomNumΪ��|AssertAndSignatureXMLΪNULL
 *		-2 - ��ȡ30����ݶ���ʧ�ܣ����Բ�����
 *		-3 - TF�����ڣ������޷���TF��
 *		-4 - TF��ǩ��ʧ��
 *		-5 - ����Pkcrypt.dllʧ��
 *      -10 - ���ǩ��ʧ��
 */
//
DLL_API int GetSignatureAndAssertXR(OUT char** AssertAndSignatureXML, OUT unsigned int *iXmlLen, IN const char* randomNum);

/* ���ƣ�FreeBuf
 * �������ͷ���DLL������ڴ�
 * ������
 *		info - ����(ָ���ַ),
 *		iLen - ���ݳ��ȣ�
 */
DLL_API void FreeBuf(char** info, unsigned int iLen);


#endif
