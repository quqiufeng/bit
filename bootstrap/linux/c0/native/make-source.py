import re,sys
s=open(sys.argv[1]).read()
def body_replace(s, sig, new):
    i=s.find(sig)
    if i<0: raise SystemExit('missing '+sig)
    b=s.find('{',i); d=0; e=b
    while e<len(s):
        if s[e]=='{': d+=1
        elif s[e]=='}':
            d-=1
            if d==0: break
        e+=1
    return s[:b]+new+s[e+1:]
s=body_replace(s,'unsigned long long fpe_arithmetic', '{ return fpe_asm ( type , op , a , b ) ; }')
s=body_replace(s,'int sc_complex_wide', '''{ unsigned long long v [ 6 ] ; v [ 0 ] = a ; v [ 1 ] = b ; v [ 2 ] = c ; v [ 3 ] = d ; v [ 4 ] = 0 ; v [ 5 ] = 0 ; sc_wide_asm ( v , t , op ) ; if ( active && ( ! fpe_finite ( t , v [ 4 ] ) || ! fpe_finite ( t , v [ 5 ] ) ) ) error_message ( "complex constant arithmetic out of range" ) ; sc_bits = v [ 4 ] ; sc_imaginary = v [ 5 ] ; return 0 ; }''')
# typedefs and local function pointers
for pat in [r'typedef\s+long\s+long\s+DiagnosticHandle\s*\([^;]*\)\s*;',
            r'typedef\s+int\s+DiagnosticWrite\s*\([^;]*\)\s*;',
            r'typedef\s+void\s+PP_LocalTime\s*\([^;]*\)\s*;',
            r'typedef\s+long\s+long\s+PP_Open\s*\([^;]*\)\s*;',
            r'typedef\s+int\s+PP_Read\s*\([^;]*\)\s*;',
            r'typedef\s+int\s+PP_Close\s*\([^;]*\)\s*;',
            r'typedef\s+unsigned\s+int\s+PP_Environment\s*\([^;]*\)\s*;',
            r'typedef\s+int\s+Lead\s*\([^;]*\)\s*;',
            r'typedef\s+void\s*\*\s*Heap\s*\([^;]*\)\s*;',
            r'typedef\s+void\s*\*\s*Allocate\s*\([^;]*\)\s*;',
            r'typedef\s+int\s+Release\s*\([^;]*\)\s*;']:
    s=re.sub(pat,'',s)
for name,var in [('DiagnosticHandle','get'),('DiagnosticWrite','write'),('PP_Open','open'),('PP_Environment','environment'),('PP_Read','read'),('PP_Close','close'),('Lead','lead'),('Heap','heap'),('Allocate','allocate'),('Release','release')]:
    s=re.sub(r'\b'+name+r'\s*\*\s*'+var+r'[^;]*;', '', s)
s=re.sub(r'int\s+pp_clock_read\s*\(\s*PP_LocalTime\s*\*\s*local_time\s*\)', 'int pp_clock_read ( void )', s)
s=s.replace('local_time ( clock_value )','__c1_linux_GetLocalTime ( clock_value )')
s=re.sub(r'pp_clock_read\s*\(\s*\(\s*PP_LocalTime\s*\*\s*\)\s*__c1_linux_GetLocalTime\s*\)','pp_clock_read ( )',s)
s=s.replace('lead ( 0 ,','__c1_linux_IsDBCSLeadByteEx ( 0 ,')
s=s.replace('return open (','return __c1_linux_CreateFileA (')
s=re.sub(r'(?<![A-Za-z0-9_])environment\s*\(', '__c1_linux_GetEnvironmentVariableA (', s)
s=s.replace('get ( - 11 )','__c1_linux_GetStdHandle ( - 11 )')
s=re.sub(r'(?<![A-Za-z0-9_])write\s*\(\s*handle\s*,', '__c1_linux_WriteFile ( handle ,', s)
s=re.sub(r'(?<![A-Za-z0-9_])close\s*\(\s*handle\s*\)', '__c1_linux_CloseHandle ( handle )', s)
s=re.sub(r'(?<![A-Za-z0-9_])read\s*\(', '__c1_linux_ReadFile (', s)
s=s.replace('if ( ! close ( handle ) )','if ( ! __c1_linux_CloseHandle ( handle ) )')
s=s.replace('p = allocate ( heap ( ) , 0 ,','p = __c1_linux_HeapAlloc ( __c1_linux_GetProcessHeap ( ) , 0 ,')
s=s.replace('release ( heap ( ) , 0 , p )','__c1_linux_HeapFree ( __c1_linux_GetProcessHeap ( ) , 0 , p )')
for var in ['get','write','open','environment','read','close','lead','heap','allocate','release']:
    s=re.sub(r'\b'+var+r'\s*=\s*\([^;]*;', '', s)
s=s.replace('return names [ id * 256 + ( * at ) ++ ] ;','{ int _idx = * at ; * at = _idx + 1 ; return names [ id * 256 + _idx ] ; }')
s=re.sub(r"([A-Za-z_][A-Za-z0-9_]*\s*\[\s*)\(\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)\s*\+\+\s*(\])\s*=\s*([^;]+);", r"\1*\2\3 = \4; *\2 = *\2 + 1;", s)
open(sys.argv[2],'w').write(s)
