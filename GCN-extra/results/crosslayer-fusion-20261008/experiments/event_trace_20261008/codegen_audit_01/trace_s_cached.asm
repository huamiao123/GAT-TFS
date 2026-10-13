0000000000463c70 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)>:
  463c70:	55                   	push   %rbp
  463c71:	41 57                	push   %r15
  463c73:	41 56                	push   %r14
  463c75:	41 55                	push   %r13
  463c77:	41 54                	push   %r12
  463c79:	53                   	push   %rbx
  463c7a:	0f b6 44 24 50       	movzbl 0x50(%rsp),%eax
  463c7f:	8b 6c 24 48          	mov    0x48(%rsp),%ebp
  463c83:	44 8b 7c 24 40       	mov    0x40(%rsp),%r15d
  463c88:	44 8b 64 24 38       	mov    0x38(%rsp),%r12d
  463c8d:	48 8b 5c 24 78       	mov    0x78(%rsp),%rbx
  463c92:	4c 8b 13             	mov    (%rbx),%r10
  463c95:	4d 8d 5a 01          	lea    0x1(%r10),%r11
  463c99:	4c 89 5c 24 e8       	mov    %r11,-0x18(%rsp)
  463c9e:	4c 89 1b             	mov    %r11,(%rbx)
  463ca1:	45 0f b6 1a          	movzbl (%r10),%r11d
  463ca5:	4d 89 dd             	mov    %r11,%r13
  463ca8:	41 83 c3 03          	add    $0x3,%r11d
  463cac:	41 c1 eb 02          	shr    $0x2,%r11d
  463cb0:	4f 8d 54 1a 01       	lea    0x1(%r10,%r11,1),%r10
  463cb5:	4c 89 13             	mov    %r10,(%rbx)
  463cb8:	4d 63 d0             	movslq %r8d,%r10
  463cbb:	4c 89 54 24 d0       	mov    %r10,-0x30(%rsp)
  463cc0:	46 8b 14 91          	mov    (%rcx,%r10,4),%r10d
  463cc4:	41 39 ea             	cmp    %ebp,%r10d
  463cc7:	44 0f 43 d5          	cmovae %ebp,%r10d
  463ccb:	45 39 fa             	cmp    %r15d,%r10d
  463cce:	45 89 fb             	mov    %r15d,%r11d
  463cd1:	44 89 54 24 cc       	mov    %r10d,-0x34(%rsp)
  463cd6:	45 0f 42 da          	cmovb  %r10d,%r11d
  463cda:	44 89 5c 24 c8       	mov    %r11d,-0x38(%rsp)
  463cdf:	45 39 e3             	cmp    %r12d,%r11d
  463ce2:	41 89 c3             	mov    %eax,%r11d
  463ce5:	41 0f 97 c2          	seta   %r10b
  463ce9:	45 08 d3             	or     %r10b,%r11b
  463cec:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  463cf1:	45 85 c9             	test   %r9d,%r9d
  463cf4:	78 64                	js     463d5a <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xea>
  463cf6:	45 89 ce             	mov    %r9d,%r14d
  463cf9:	42 8b 0c b1          	mov    (%rcx,%r14,4),%ecx
  463cfd:	39 e9                	cmp    %ebp,%ecx
  463cff:	0f 42 e9             	cmovb  %ecx,%ebp
  463d02:	44 39 fd             	cmp    %r15d,%ebp
  463d05:	89 6c 24 b8          	mov    %ebp,-0x48(%rsp)
  463d09:	44 0f 42 fd          	cmovb  %ebp,%r15d
  463d0d:	44 89 7c 24 bc       	mov    %r15d,-0x44(%rsp)
  463d12:	45 39 e7             	cmp    %r12d,%r15d
  463d15:	0f 97 c1             	seta   %cl
  463d18:	08 c1                	or     %al,%cl
  463d1a:	45 84 db             	test   %r11b,%r11b
  463d1d:	0f 84 89 00 00 00    	je     463dac <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x13c>
  463d23:	84 c0                	test   %al,%al
  463d25:	0f 84 8e 00 00 00    	je     463db9 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x149>
  463d2b:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  463d30:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  463d35:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  463d39:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  463d3d:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  463d41:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  463d45:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  463d49:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  463d4d:	84 c9                	test   %cl,%cl
  463d4f:	0f 85 b7 00 00 00    	jne    463e0c <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x19c>
  463d55:	e9 f9 00 00 00       	jmpq   463e53 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1e3>
  463d5a:	45 84 db             	test   %r11b,%r11b
  463d5d:	0f 84 1a 0a 00 00    	je     46477d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xb0d>
  463d63:	84 c0                	test   %al,%al
  463d65:	0f 84 8f 01 00 00    	je     463efa <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x28a>
  463d6b:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  463d70:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  463d75:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  463d79:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  463d7d:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  463d81:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  463d85:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  463d89:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  463d8d:	c6 44 24 b7 01       	movb   $0x1,-0x49(%rsp)
  463d92:	44 39 64 24 c8       	cmp    %r12d,-0x38(%rsp)
  463d97:	0f 87 b4 01 00 00    	ja     463f51 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2e1>
  463d9d:	c7 44 24 b8 00 00 00 	movl   $0x0,-0x48(%rsp)
  463da4:	00 
  463da5:	31 c0                	xor    %eax,%eax
  463da7:	e9 c2 01 00 00       	jmpq   463f6e <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x2fe>
  463dac:	84 c9                	test   %cl,%cl
  463dae:	0f 84 c9 09 00 00    	je     46477d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xb0d>
  463db4:	45 31 d2             	xor    %r10d,%r10d
  463db7:	eb 53                	jmp    463e0c <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x19c>
  463db9:	44 89 dd             	mov    %r11d,%ebp
  463dbc:	45 89 c3             	mov    %r8d,%r11d
  463dbf:	41 c1 e3 07          	shl    $0x7,%r11d
  463dc3:	4d 63 db             	movslq %r11d,%r11
  463dc6:	62 b1 7c 48 10 44 9b 	vmovups 0x1c0(%rbx,%r11,4),%zmm0
  463dcd:	07 
  463dce:	62 b1 7c 48 10 4c 9b 	vmovups 0x180(%rbx,%r11,4),%zmm1
  463dd5:	06 
  463dd6:	62 b1 7c 48 10 54 9b 	vmovups 0x140(%rbx,%r11,4),%zmm2
  463ddd:	05 
  463dde:	62 b1 7c 48 10 5c 9b 	vmovups 0x100(%rbx,%r11,4),%zmm3
  463de5:	04 
  463de6:	62 31 7c 48 10 0c 9b 	vmovups (%rbx,%r11,4),%zmm9
  463ded:	62 31 7c 48 10 44 9b 	vmovups 0x40(%rbx,%r11,4),%zmm8
  463df4:	01 
  463df5:	62 b1 7c 48 10 74 9b 	vmovups 0x80(%rbx,%r11,4),%zmm6
  463dfc:	02 
  463dfd:	62 b1 7c 48 10 64 9b 	vmovups 0xc0(%rbx,%r11,4),%zmm4
  463e04:	03 
  463e05:	41 89 eb             	mov    %ebp,%r11d
  463e08:	84 c9                	test   %cl,%cl
  463e0a:	74 47                	je     463e53 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1e3>
  463e0c:	b1 01                	mov    $0x1,%cl
  463e0e:	89 4c 24 c0          	mov    %ecx,-0x40(%rsp)
  463e12:	84 c0                	test   %al,%al
  463e14:	44 88 5c 24 b7       	mov    %r11b,-0x49(%rsp)
  463e19:	74 54                	je     463e6f <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x1ff>
  463e1b:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  463e20:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  463e25:	c4 41 08 57 f6       	vxorps %xmm14,%xmm14,%xmm14
  463e2a:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  463e2f:	c4 41 20 57 db       	vxorps %xmm11,%xmm11,%xmm11
  463e34:	c4 41 28 57 d2       	vxorps %xmm10,%xmm10,%xmm10
  463e39:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  463e3d:	c5 d0 57 ed          	vxorps %xmm5,%xmm5,%xmm5
  463e41:	45 31 db             	xor    %r11d,%r11d
  463e44:	b8 00 00 00 00       	mov    $0x0,%eax
  463e49:	45 84 d2             	test   %r10b,%r10b
  463e4c:	75 75                	jne    463ec3 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x253>
  463e4e:	e9 85 00 00 00       	jmpq   463ed8 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x268>
  463e53:	c6 44 24 b7 01       	movb   $0x1,-0x49(%rsp)
  463e58:	c7 44 24 c0 00 00 00 	movl   $0x0,-0x40(%rsp)
  463e5f:	00 
  463e60:	45 31 db             	xor    %r11d,%r11d
  463e63:	b8 00 00 00 00       	mov    $0x0,%eax
  463e68:	45 84 d2             	test   %r10b,%r10b
  463e6b:	75 56                	jne    463ec3 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x253>
  463e6d:	eb 69                	jmp    463ed8 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x268>
  463e6f:	44 89 c8             	mov    %r9d,%eax
  463e72:	c1 e0 07             	shl    $0x7,%eax
  463e75:	48 98                	cltq   
  463e77:	62 f1 7c 48 10 6c 83 	vmovups 0x1c0(%rbx,%rax,4),%zmm5
  463e7e:	07 
  463e7f:	62 f1 7c 48 10 7c 83 	vmovups 0x180(%rbx,%rax,4),%zmm7
  463e86:	06 
  463e87:	62 71 7c 48 10 54 83 	vmovups 0x140(%rbx,%rax,4),%zmm10
  463e8e:	05 
  463e8f:	62 71 7c 48 10 5c 83 	vmovups 0x100(%rbx,%rax,4),%zmm11
  463e96:	04 
  463e97:	62 71 7c 48 10 2c 83 	vmovups (%rbx,%rax,4),%zmm13
  463e9e:	62 71 7c 48 10 7c 83 	vmovups 0x40(%rbx,%rax,4),%zmm15
  463ea5:	01 
  463ea6:	62 71 7c 48 10 74 83 	vmovups 0x80(%rbx,%rax,4),%zmm14
  463ead:	02 
  463eae:	62 71 7c 48 10 64 83 	vmovups 0xc0(%rbx,%rax,4),%zmm12
  463eb5:	03 
  463eb6:	45 31 db             	xor    %r11d,%r11d
  463eb9:	b8 00 00 00 00       	mov    $0x0,%eax
  463ebe:	45 84 d2             	test   %r10b,%r10b
  463ec1:	74 15                	je     463ed8 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x268>
  463ec3:	48 8b 44 24 d0       	mov    -0x30(%rsp),%rax
  463ec8:	8b 04 82             	mov    (%rdx,%rax,4),%eax
  463ecb:	44 89 e1             	mov    %r12d,%ecx
  463ece:	48 01 c1             	add    %rax,%rcx
  463ed1:	48 8b 47 18          	mov    0x18(%rdi),%rax
  463ed5:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  463ed8:	44 39 64 24 bc       	cmp    %r12d,-0x44(%rsp)
  463edd:	0f 86 9e 00 00 00    	jbe    463f81 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x311>
  463ee3:	42 8b 0c b2          	mov    (%rdx,%r14,4),%ecx
  463ee7:	45 89 e2             	mov    %r12d,%r10d
  463eea:	49 01 ca             	add    %rcx,%r10
  463eed:	48 8b 4f 18          	mov    0x18(%rdi),%rcx
  463ef1:	46 8b 1c 91          	mov    (%rcx,%r10,4),%r11d
  463ef5:	e9 87 00 00 00       	jmpq   463f81 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x311>
  463efa:	44 89 c0             	mov    %r8d,%eax
  463efd:	c1 e0 07             	shl    $0x7,%eax
  463f00:	48 98                	cltq   
  463f02:	62 f1 7c 48 10 44 83 	vmovups 0x1c0(%rbx,%rax,4),%zmm0
  463f09:	07 
  463f0a:	62 f1 7c 48 10 4c 83 	vmovups 0x180(%rbx,%rax,4),%zmm1
  463f11:	06 
  463f12:	62 f1 7c 48 10 54 83 	vmovups 0x140(%rbx,%rax,4),%zmm2
  463f19:	05 
  463f1a:	62 f1 7c 48 10 5c 83 	vmovups 0x100(%rbx,%rax,4),%zmm3
  463f21:	04 
  463f22:	62 71 7c 48 10 0c 83 	vmovups (%rbx,%rax,4),%zmm9
  463f29:	62 71 7c 48 10 44 83 	vmovups 0x40(%rbx,%rax,4),%zmm8
  463f30:	01 
  463f31:	62 f1 7c 48 10 74 83 	vmovups 0x80(%rbx,%rax,4),%zmm6
  463f38:	02 
  463f39:	62 f1 7c 48 10 64 83 	vmovups 0xc0(%rbx,%rax,4),%zmm4
  463f40:	03 
  463f41:	c6 44 24 b7 01       	movb   $0x1,-0x49(%rsp)
  463f46:	44 39 64 24 c8       	cmp    %r12d,-0x38(%rsp)
  463f4b:	0f 86 4c fe ff ff    	jbe    463d9d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x12d>
  463f51:	48 8b 44 24 d0       	mov    -0x30(%rsp),%rax
  463f56:	8b 04 82             	mov    (%rdx,%rax,4),%eax
  463f59:	44 89 e1             	mov    %r12d,%ecx
  463f5c:	48 01 c1             	add    %rax,%rcx
  463f5f:	48 8b 47 18          	mov    0x18(%rdi),%rax
  463f63:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  463f66:	c7 44 24 b8 00 00 00 	movl   $0x0,-0x48(%rsp)
  463f6d:	00 
  463f6e:	c7 44 24 bc 00 00 00 	movl   $0x0,-0x44(%rsp)
  463f75:	00 
  463f76:	c7 44 24 c0 00 00 00 	movl   $0x0,-0x40(%rsp)
  463f7d:	00 
  463f7e:	45 31 db             	xor    %r11d,%r11d
  463f81:	4c 89 e9             	mov    %r13,%rcx
  463f84:	44 89 44 24 c4       	mov    %r8d,-0x3c(%rsp)
  463f89:	85 c9                	test   %ecx,%ecx
  463f8b:	44 89 4c 24 dc       	mov    %r9d,-0x24(%rsp)
  463f90:	0f 84 99 03 00 00    	je     46432f <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x6bf>
  463f96:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  463f9a:	4d 63 c1             	movslq %r9d,%r8
  463f9d:	41 89 c9             	mov    %ecx,%r9d
  463fa0:	4c 89 4c 24 e0       	mov    %r9,-0x20(%rsp)
  463fa5:	45 31 d2             	xor    %r10d,%r10d
  463fa8:	45 89 e5             	mov    %r12d,%r13d
  463fab:	44 8b 4c 24 b8       	mov    -0x48(%rsp),%r9d
  463fb0:	eb 2a                	jmp    463fdc <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x36c>
  463fb2:	66 66 66 66 66 2e 0f 	data16 data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  463fb9:	1f 84 00 00 00 00 00 
  463fc0:	4c 8b 54 24 f0       	mov    -0x10(%rsp),%r10
  463fc5:	49 83 c2 04          	add    $0x4,%r10
  463fc9:	48 8b 4c 24 f8       	mov    -0x8(%rsp),%rcx
  463fce:	83 c1 fc             	add    $0xfffffffc,%ecx
  463fd1:	4c 3b 54 24 e0       	cmp    -0x20(%rsp),%r10
  463fd6:	0f 83 53 03 00 00    	jae    46432f <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x6bf>
  463fdc:	83 f9 04             	cmp    $0x4,%ecx
  463fdf:	41 bf 04 00 00 00    	mov    $0x4,%r15d
  463fe5:	48 89 4c 24 f8       	mov    %rcx,-0x8(%rsp)
  463fea:	44 0f 42 f9          	cmovb  %ecx,%r15d
  463fee:	41 83 ff 01          	cmp    $0x1,%r15d
  463ff2:	41 83 d7 00          	adc    $0x0,%r15d
  463ff6:	4c 89 54 24 f0       	mov    %r10,-0x10(%rsp)
  463ffb:	4c 89 d1             	mov    %r10,%rcx
  463ffe:	48 c1 e9 02          	shr    $0x2,%rcx
  464002:	4c 8b 54 24 e8       	mov    -0x18(%rsp),%r10
  464007:	41 0f b6 0c 0a       	movzbl (%r10,%rcx,1),%ecx
  46400c:	eb 0a                	jmp    464018 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3a8>
  46400e:	66 90                	xchg   %ax,%ax
  464010:	c0 e9 02             	shr    $0x2,%cl
  464013:	41 ff cf             	dec    %r15d
  464016:	74 a8                	je     463fc0 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x350>
  464018:	45 8d 74 24 01       	lea    0x1(%r12),%r14d
  46401d:	f6 c1 01             	test   $0x1,%cl
  464020:	74 3c                	je     46405e <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3ee>
  464022:	44 3b 74 24 cc       	cmp    -0x34(%rsp),%r14d
  464027:	73 35                	jae    46405e <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3ee>
  464029:	4c 8b 54 24 d0       	mov    -0x30(%rsp),%r10
  46402e:	46 8b 14 92          	mov    (%rdx,%r10,4),%r10d
  464032:	44 89 e5             	mov    %r12d,%ebp
  464035:	4c 01 d5             	add    %r10,%rbp
  464038:	44 8b 54 af 04       	mov    0x4(%rdi,%rbp,4),%r10d
  46403d:	49 c1 e2 08          	shl    $0x8,%r10
  464041:	42 0f 18 0c 16       	prefetcht0 (%rsi,%r10,1)
  464046:	42 0f 18 4c 16 40    	prefetcht0 0x40(%rsi,%r10,1)
  46404c:	42 0f 18 8c 16 80 00 	prefetcht0 0x80(%rsi,%r10,1)
  464053:	00 00 
  464055:	42 0f 18 8c 16 c0 00 	prefetcht0 0xc0(%rsi,%r10,1)
  46405c:	00 00 
  46405e:	41 89 ca             	mov    %ecx,%r10d
  464061:	41 80 e2 03          	and    $0x3,%r10b
  464065:	41 80 fa 02          	cmp    $0x2,%r10b
  464069:	0f 83 d1 00 00 00    	jae    464140 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x4d0>
  46406f:	41 80 fa 01          	cmp    $0x1,%r10b
  464073:	0f 85 b7 01 00 00    	jne    464230 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x5c0>
  464079:	89 c3                	mov    %eax,%ebx
  46407b:	48 c1 e3 08          	shl    $0x8,%rbx
  46407f:	62 e2 7d 48 33 44 1e 	vpmovzxwd 0xa0(%rsi,%rbx,1),%zmm16
  464086:	05 
  464087:	62 e2 7d 48 33 4c 1e 	vpmovzxwd 0x80(%rsi,%rbx,1),%zmm17
  46408e:	04 
  46408f:	62 e2 7d 48 33 54 1e 	vpmovzxwd 0x60(%rsi,%rbx,1),%zmm18
  464096:	03 
  464097:	62 e2 7d 48 33 5c 1e 	vpmovzxwd 0x40(%rsi,%rbx,1),%zmm19
  46409e:	02 
  46409f:	62 e2 7d 48 33 64 1e 	vpmovzxwd 0x20(%rsi,%rbx,1),%zmm20
  4640a6:	01 
  4640a7:	62 e2 7d 48 33 2c 1e 	vpmovzxwd (%rsi,%rbx,1),%zmm21
  4640ae:	62 e2 7d 48 33 74 1e 	vpmovzxwd 0xc0(%rsi,%rbx,1),%zmm22
  4640b5:	06 
  4640b6:	62 e2 7d 48 33 7c 1e 	vpmovzxwd 0xe0(%rsi,%rbx,1),%zmm23
  4640bd:	07 
  4640be:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  4640c5:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  4640cc:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  4640d3:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  4640da:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  4640e1:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  4640e8:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  4640ef:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  4640f6:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  4640fc:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  464102:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  464108:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  46410e:	62 31 3c 48 58 c4    	vaddps %zmm20,%zmm8,%zmm8
  464114:	62 31 34 48 58 cd    	vaddps %zmm21,%zmm9,%zmm9
  46411a:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  464120:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  464126:	f6 c1 01             	test   $0x1,%cl
  464129:	0f 85 b4 01 00 00    	jne    4642e3 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x673>
  46412f:	e9 d0 01 00 00       	jmpq   464304 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x694>
  464134:	66 66 66 2e 0f 1f 84 	data16 data16 nopw %cs:0x0(%rax,%rax,1)
  46413b:	00 00 00 00 00 
  464140:	41 8d 6d 01          	lea    0x1(%r13),%ebp
  464144:	44 39 cd             	cmp    %r9d,%ebp
  464147:	73 2b                	jae    464174 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x504>
  464149:	42 8b 2c 82          	mov    (%rdx,%r8,4),%ebp
  46414d:	44 89 eb             	mov    %r13d,%ebx
  464150:	48 01 eb             	add    %rbp,%rbx
  464153:	8b 5c 9f 04          	mov    0x4(%rdi,%rbx,4),%ebx
  464157:	48 c1 e3 08          	shl    $0x8,%rbx
  46415b:	0f 18 0c 1e          	prefetcht0 (%rsi,%rbx,1)
  46415f:	0f 18 4c 1e 40       	prefetcht0 0x40(%rsi,%rbx,1)
  464164:	0f 18 8c 1e 80 00 00 	prefetcht0 0x80(%rsi,%rbx,1)
  46416b:	00 
  46416c:	0f 18 8c 1e c0 00 00 	prefetcht0 0xc0(%rsi,%rbx,1)
  464173:	00 
  464174:	41 80 fa 03          	cmp    $0x3,%r10b
  464178:	0f 85 b2 00 00 00    	jne    464230 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x5c0>
  46417e:	89 c3                	mov    %eax,%ebx
  464180:	48 c1 e3 08          	shl    $0x8,%rbx
  464184:	62 e2 7d 48 33 44 1e 	vpmovzxwd 0xa0(%rsi,%rbx,1),%zmm16
  46418b:	05 
  46418c:	62 e2 7d 48 33 4c 1e 	vpmovzxwd 0x80(%rsi,%rbx,1),%zmm17
  464193:	04 
  464194:	62 e2 7d 48 33 54 1e 	vpmovzxwd 0x60(%rsi,%rbx,1),%zmm18
  46419b:	03 
  46419c:	62 e2 7d 48 33 5c 1e 	vpmovzxwd 0x40(%rsi,%rbx,1),%zmm19
  4641a3:	02 
  4641a4:	62 e2 7d 48 33 64 1e 	vpmovzxwd 0x20(%rsi,%rbx,1),%zmm20
  4641ab:	01 
  4641ac:	62 e2 7d 48 33 2c 1e 	vpmovzxwd (%rsi,%rbx,1),%zmm21
  4641b3:	62 e2 7d 48 33 74 1e 	vpmovzxwd 0xc0(%rsi,%rbx,1),%zmm22
  4641ba:	06 
  4641bb:	62 e2 7d 48 33 7c 1e 	vpmovzxwd 0xe0(%rsi,%rbx,1),%zmm23
  4641c2:	07 
  4641c3:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  4641ca:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  4641d1:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  4641d8:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  4641df:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  4641e6:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  4641ed:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  4641f4:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  4641fb:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  464201:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  464207:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  46420d:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  464213:	62 31 3c 48 58 c4    	vaddps %zmm20,%zmm8,%zmm8
  464219:	62 31 34 48 58 cd    	vaddps %zmm21,%zmm9,%zmm9
  46421f:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  464225:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  46422b:	e9 7e 00 00 00       	jmpq   4642ae <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x63e>
  464230:	44 89 db             	mov    %r11d,%ebx
  464233:	48 c1 e3 08          	shl    $0x8,%rbx
  464237:	62 e2 7d 48 33 44 1e 	vpmovzxwd 0xa0(%rsi,%rbx,1),%zmm16
  46423e:	05 
  46423f:	62 e2 7d 48 33 4c 1e 	vpmovzxwd 0x80(%rsi,%rbx,1),%zmm17
  464246:	04 
  464247:	62 e2 7d 48 33 54 1e 	vpmovzxwd 0x60(%rsi,%rbx,1),%zmm18
  46424e:	03 
  46424f:	62 e2 7d 48 33 5c 1e 	vpmovzxwd 0x40(%rsi,%rbx,1),%zmm19
  464256:	02 
  464257:	62 e2 7d 48 33 64 1e 	vpmovzxwd 0x20(%rsi,%rbx,1),%zmm20
  46425e:	01 
  46425f:	62 e2 7d 48 33 2c 1e 	vpmovzxwd (%rsi,%rbx,1),%zmm21
  464266:	62 e2 7d 48 33 74 1e 	vpmovzxwd 0xc0(%rsi,%rbx,1),%zmm22
  46426d:	06 
  46426e:	62 e2 7d 48 33 7c 1e 	vpmovzxwd 0xe0(%rsi,%rbx,1),%zmm23
  464275:	07 
  464276:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46427d:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  464284:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  46428b:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  464292:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  464299:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  4642a0:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  4642a7:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  4642ae:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  4642b4:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  4642ba:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  4642c0:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  4642c6:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  4642cc:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  4642d2:	62 b1 44 48 58 fe    	vaddps %zmm22,%zmm7,%zmm7
  4642d8:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  4642de:	f6 c1 01             	test   $0x1,%cl
  4642e1:	74 21                	je     464304 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x694>
  4642e3:	44 3b 74 24 c8       	cmp    -0x38(%rsp),%r14d
  4642e8:	73 17                	jae    464301 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x691>
  4642ea:	48 8b 44 24 d0       	mov    -0x30(%rsp),%rax
  4642ef:	8b 04 82             	mov    (%rdx,%rax,4),%eax
  4642f2:	45 89 f1             	mov    %r14d,%r9d
  4642f5:	49 01 c1             	add    %rax,%r9
  4642f8:	42 8b 04 8f          	mov    (%rdi,%r9,4),%eax
  4642fc:	44 8b 4c 24 b8       	mov    -0x48(%rsp),%r9d
  464301:	45 89 f4             	mov    %r14d,%r12d
  464304:	41 80 fa 02          	cmp    $0x2,%r10b
  464308:	0f 82 02 fd ff ff    	jb     464010 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3a0>
  46430e:	41 ff c5             	inc    %r13d
  464311:	44 3b 6c 24 bc       	cmp    -0x44(%rsp),%r13d
  464316:	0f 83 f4 fc ff ff    	jae    464010 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3a0>
  46431c:	46 8b 14 82          	mov    (%rdx,%r8,4),%r10d
  464320:	45 89 eb             	mov    %r13d,%r11d
  464323:	4d 01 d3             	add    %r10,%r11
  464326:	46 8b 1c 9f          	mov    (%rdi,%r11,4),%r11d
  46432a:	e9 e1 fc ff ff       	jmpq   464010 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x3a0>
  46432f:	48 8b 4c 24 70       	mov    0x70(%rsp),%rcx
  464334:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  464339:	48 8b 54 24 60       	mov    0x60(%rsp),%rdx
  46433e:	80 7c 24 b7 00       	cmpb   $0x0,-0x49(%rsp)
  464343:	8b 7c 24 dc          	mov    -0x24(%rsp),%edi
  464347:	4c 8b 44 24 58       	mov    0x58(%rsp),%r8
  46434c:	0f 84 c0 01 00 00    	je     464512 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x8a2>
  464352:	80 79 2a 00          	cmpb   $0x0,0x2a(%rcx)
  464356:	0f 84 86 03 00 00    	je     4646e2 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa72>
  46435c:	8b 74 24 cc          	mov    -0x34(%rsp),%esi
  464360:	3b 74 24 40          	cmp    0x40(%rsp),%esi
  464364:	0f 87 78 03 00 00    	ja     4646e2 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa72>
  46436a:	62 d1 7d 40 72 d1 10 	vpsrld $0x10,%zmm9,%zmm16
  464371:	8b 74 24 c4          	mov    -0x3c(%rsp),%esi
  464375:	c1 e6 07             	shl    $0x7,%esi
  464378:	48 63 f6             	movslq %esi,%rsi
  46437b:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  46437f:	62 e2 7e 48 33 04 72 	vpmovdw %zmm16,(%rdx,%rsi,2)
  464386:	74 1e                	je     4643a6 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x736>
  464388:	62 e1 34 58 54 05 66 	vandps 0x41766(%rip){1to16},%zmm9,%zmm16        # 4a5af8 <__dso_handle+0x30>
  46438f:	17 04 00 
  464392:	62 31 34 48 5c c8    	vsubps %zmm16,%zmm9,%zmm9
  464398:	62 d1 35 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm9
  46439f:	62 72 7e 48 33 0c 70 	vpmovdw %zmm9,(%rax,%rsi,2)
  4643a6:	62 d1 35 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm9
  4643ad:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4643b1:	62 72 7e 48 33 4c 72 	vpmovdw %zmm9,0x20(%rdx,%rsi,2)
  4643b8:	01 
  4643b9:	74 1f                	je     4643da <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x76a>
  4643bb:	62 71 3c 58 54 0d 33 	vandps 0x41733(%rip){1to16},%zmm8,%zmm9        # 4a5af8 <__dso_handle+0x30>
  4643c2:	17 04 00 
  4643c5:	62 51 3c 48 5c c1    	vsubps %zmm9,%zmm8,%zmm8
  4643cb:	62 d1 3d 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm8
  4643d2:	62 72 7e 48 33 44 70 	vpmovdw %zmm8,0x20(%rax,%rsi,2)
  4643d9:	01 
  4643da:	62 f1 3d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm8
  4643e1:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4643e5:	62 72 7e 48 33 44 72 	vpmovdw %zmm8,0x40(%rdx,%rsi,2)
  4643ec:	02 
  4643ed:	74 1f                	je     46440e <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x79e>
  4643ef:	62 71 4c 58 54 05 ff 	vandps 0x416ff(%rip){1to16},%zmm6,%zmm8        # 4a5af8 <__dso_handle+0x30>
  4643f6:	16 04 00 
  4643f9:	62 d1 4c 48 5c f0    	vsubps %zmm8,%zmm6,%zmm6
  4643ff:	62 f1 4d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm6
  464406:	62 f2 7e 48 33 74 70 	vpmovdw %zmm6,0x40(%rax,%rsi,2)
  46440d:	02 
  46440e:	62 f1 4d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm6
  464415:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464419:	62 f2 7e 48 33 74 72 	vpmovdw %zmm6,0x60(%rdx,%rsi,2)
  464420:	03 
  464421:	74 1f                	je     464442 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x7d2>
  464423:	62 f1 5c 58 54 35 cb 	vandps 0x416cb(%rip){1to16},%zmm4,%zmm6        # 4a5af8 <__dso_handle+0x30>
  46442a:	16 04 00 
  46442d:	62 f1 5c 48 5c e6    	vsubps %zmm6,%zmm4,%zmm4
  464433:	62 f1 5d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm4
  46443a:	62 f2 7e 48 33 64 70 	vpmovdw %zmm4,0x60(%rax,%rsi,2)
  464441:	03 
  464442:	62 f1 5d 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm4
  464449:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  46444d:	62 f2 7e 48 33 64 72 	vpmovdw %zmm4,0x80(%rdx,%rsi,2)
  464454:	04 
  464455:	74 1f                	je     464476 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x806>
  464457:	62 f1 64 58 54 25 97 	vandps 0x41697(%rip){1to16},%zmm3,%zmm4        # 4a5af8 <__dso_handle+0x30>
  46445e:	16 04 00 
  464461:	62 f1 64 48 5c dc    	vsubps %zmm4,%zmm3,%zmm3
  464467:	62 f1 65 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm3
  46446e:	62 f2 7e 48 33 5c 70 	vpmovdw %zmm3,0x80(%rax,%rsi,2)
  464475:	04 
  464476:	62 f1 65 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm3
  46447d:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464481:	62 f2 7e 48 33 5c 72 	vpmovdw %zmm3,0xa0(%rdx,%rsi,2)
  464488:	05 
  464489:	74 1f                	je     4644aa <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x83a>
  46448b:	62 f1 6c 58 54 1d 63 	vandps 0x41663(%rip){1to16},%zmm2,%zmm3        # 4a5af8 <__dso_handle+0x30>
  464492:	16 04 00 
  464495:	62 f1 6c 48 5c d3    	vsubps %zmm3,%zmm2,%zmm2
  46449b:	62 f1 6d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm2
  4644a2:	62 f2 7e 48 33 54 70 	vpmovdw %zmm2,0xa0(%rax,%rsi,2)
  4644a9:	05 
  4644aa:	62 f1 6d 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm2
  4644b1:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4644b5:	62 f2 7e 48 33 54 72 	vpmovdw %zmm2,0xc0(%rdx,%rsi,2)
  4644bc:	06 
  4644bd:	74 1f                	je     4644de <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x86e>
  4644bf:	62 f1 74 58 54 15 2f 	vandps 0x4162f(%rip){1to16},%zmm1,%zmm2        # 4a5af8 <__dso_handle+0x30>
  4644c6:	16 04 00 
  4644c9:	62 f1 74 48 5c ca    	vsubps %zmm2,%zmm1,%zmm1
  4644cf:	62 f1 75 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm1
  4644d6:	62 f2 7e 48 33 4c 70 	vpmovdw %zmm1,0xc0(%rax,%rsi,2)
  4644dd:	06 
  4644de:	62 f1 75 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm1
  4644e5:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4644e9:	62 f2 7e 48 33 4c 72 	vpmovdw %zmm1,0xe0(%rdx,%rsi,2)
  4644f0:	07 
  4644f1:	74 1f                	je     464512 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x8a2>
  4644f3:	62 f1 7c 58 54 0d fb 	vandps 0x415fb(%rip){1to16},%zmm0,%zmm1        # 4a5af8 <__dso_handle+0x30>
  4644fa:	15 04 00 
  4644fd:	62 f1 7c 48 5c c1    	vsubps %zmm1,%zmm0,%zmm0
  464503:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46450a:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xe0(%rax,%rsi,2)
  464511:	07 
  464512:	80 7c 24 c0 00       	cmpb   $0x0,-0x40(%rsp)
  464517:	0f 84 60 02 00 00    	je     46477d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xb0d>
  46451d:	80 79 2a 00          	cmpb   $0x0,0x2a(%rcx)
  464521:	0f 84 11 02 00 00    	je     464738 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xac8>
  464527:	8b 74 24 bc          	mov    -0x44(%rsp),%esi
  46452b:	3b 74 24 b8          	cmp    -0x48(%rsp),%esi
  46452f:	0f 85 03 02 00 00    	jne    464738 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xac8>
  464535:	62 d1 7d 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm0
  46453c:	c1 e7 07             	shl    $0x7,%edi
  46453f:	48 63 f7             	movslq %edi,%rsi
  464542:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464546:	62 f2 7e 48 33 04 72 	vpmovdw %zmm0,(%rdx,%rsi,2)
  46454d:	74 1e                	je     46456d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x8fd>
  46454f:	62 f1 14 58 54 05 9f 	vandps 0x4159f(%rip){1to16},%zmm13,%zmm0        # 4a5af8 <__dso_handle+0x30>
  464556:	15 04 00 
  464559:	62 f1 14 48 5c c0    	vsubps %zmm0,%zmm13,%zmm0
  46455f:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464566:	62 f2 7e 48 33 04 70 	vpmovdw %zmm0,(%rax,%rsi,2)
  46456d:	62 d1 7d 48 72 d7 10 	vpsrld $0x10,%zmm15,%zmm0
  464574:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464578:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x20(%rdx,%rsi,2)
  46457f:	01 
  464580:	74 1f                	je     4645a1 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x931>
  464582:	62 f1 04 58 54 05 6c 	vandps 0x4156c(%rip){1to16},%zmm15,%zmm0        # 4a5af8 <__dso_handle+0x30>
  464589:	15 04 00 
  46458c:	62 f1 04 48 5c c0    	vsubps %zmm0,%zmm15,%zmm0
  464592:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464599:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x20(%rax,%rsi,2)
  4645a0:	01 
  4645a1:	62 d1 7d 48 72 d6 10 	vpsrld $0x10,%zmm14,%zmm0
  4645a8:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4645ac:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x40(%rdx,%rsi,2)
  4645b3:	02 
  4645b4:	74 1f                	je     4645d5 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x965>
  4645b6:	62 f1 0c 58 54 05 38 	vandps 0x41538(%rip){1to16},%zmm14,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4645bd:	15 04 00 
  4645c0:	62 f1 0c 48 5c c0    	vsubps %zmm0,%zmm14,%zmm0
  4645c6:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4645cd:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x40(%rax,%rsi,2)
  4645d4:	02 
  4645d5:	62 d1 7d 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm0
  4645dc:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4645e0:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x60(%rdx,%rsi,2)
  4645e7:	03 
  4645e8:	74 1f                	je     464609 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x999>
  4645ea:	62 f1 1c 58 54 05 04 	vandps 0x41504(%rip){1to16},%zmm12,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4645f1:	15 04 00 
  4645f4:	62 f1 1c 48 5c c0    	vsubps %zmm0,%zmm12,%zmm0
  4645fa:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464601:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x60(%rax,%rsi,2)
  464608:	03 
  464609:	62 d1 7d 48 72 d3 10 	vpsrld $0x10,%zmm11,%zmm0
  464610:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464614:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0x80(%rdx,%rsi,2)
  46461b:	04 
  46461c:	74 1f                	je     46463d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x9cd>
  46461e:	62 f1 24 58 54 05 d0 	vandps 0x414d0(%rip){1to16},%zmm11,%zmm0        # 4a5af8 <__dso_handle+0x30>
  464625:	14 04 00 
  464628:	62 f1 24 48 5c c0    	vsubps %zmm0,%zmm11,%zmm0
  46462e:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464635:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0x80(%rax,%rsi,2)
  46463c:	04 
  46463d:	62 d1 7d 48 72 d2 10 	vpsrld $0x10,%zmm10,%zmm0
  464644:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  464648:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0xa0(%rdx,%rsi,2)
  46464f:	05 
  464650:	74 1f                	je     464671 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa01>
  464652:	62 f1 2c 58 54 05 9c 	vandps 0x4149c(%rip){1to16},%zmm10,%zmm0        # 4a5af8 <__dso_handle+0x30>
  464659:	14 04 00 
  46465c:	62 f1 2c 48 5c c0    	vsubps %zmm0,%zmm10,%zmm0
  464662:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  464669:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xa0(%rax,%rsi,2)
  464670:	05 
  464671:	62 f1 7d 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm0
  464678:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  46467c:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0xc0(%rdx,%rsi,2)
  464683:	06 
  464684:	74 1f                	je     4646a5 <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xa35>
  464686:	62 f1 44 58 54 05 68 	vandps 0x41468(%rip){1to16},%zmm7,%zmm0        # 4a5af8 <__dso_handle+0x30>
  46468d:	14 04 00 
  464690:	62 f1 44 48 5c c0    	vsubps %zmm0,%zmm7,%zmm0
  464696:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46469d:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xc0(%rax,%rsi,2)
  4646a4:	06 
  4646a5:	62 f1 7d 48 72 d5 10 	vpsrld $0x10,%zmm5,%zmm0
  4646ac:	80 79 28 00          	cmpb   $0x0,0x28(%rcx)
  4646b0:	62 f2 7e 48 33 44 72 	vpmovdw %zmm0,0xe0(%rdx,%rsi,2)
  4646b7:	07 
  4646b8:	0f 84 bf 00 00 00    	je     46477d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xb0d>
  4646be:	62 f1 54 58 54 05 30 	vandps 0x41430(%rip){1to16},%zmm5,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4646c5:	14 04 00 
  4646c8:	62 f1 54 48 5c c0    	vsubps %zmm0,%zmm5,%zmm0
  4646ce:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4646d5:	62 f2 7e 48 33 44 70 	vpmovdw %zmm0,0xe0(%rax,%rsi,2)
  4646dc:	07 
  4646dd:	e9 9b 00 00 00       	jmpq   46477d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xb0d>
  4646e2:	8b 74 24 c4          	mov    -0x3c(%rsp),%esi
  4646e6:	c1 e6 07             	shl    $0x7,%esi
  4646e9:	48 63 f6             	movslq %esi,%rsi
  4646ec:	62 d1 7c 48 11 74 b0 	vmovups %zmm6,0x80(%r8,%rsi,4)
  4646f3:	02 
  4646f4:	62 51 7c 48 11 44 b0 	vmovups %zmm8,0x40(%r8,%rsi,4)
  4646fb:	01 
  4646fc:	62 51 7c 48 11 0c b0 	vmovups %zmm9,(%r8,%rsi,4)
  464703:	62 d1 7c 48 11 64 b0 	vmovups %zmm4,0xc0(%r8,%rsi,4)
  46470a:	03 
  46470b:	62 d1 7c 48 11 5c b0 	vmovups %zmm3,0x100(%r8,%rsi,4)
  464712:	04 
  464713:	62 d1 7c 48 11 54 b0 	vmovups %zmm2,0x140(%r8,%rsi,4)
  46471a:	05 
  46471b:	62 d1 7c 48 11 4c b0 	vmovups %zmm1,0x180(%r8,%rsi,4)
  464722:	06 
  464723:	62 d1 7c 48 11 44 b0 	vmovups %zmm0,0x1c0(%r8,%rsi,4)
  46472a:	07 
  46472b:	80 7c 24 c0 00       	cmpb   $0x0,-0x40(%rsp)
  464730:	0f 85 e7 fd ff ff    	jne    46451d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0x8ad>
  464736:	eb 45                	jmp    46477d <void gcn_extra_event_trace::trace_pair_window<false, 1>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, unsigned char const*&, gcn_extra_event_trace::Profile&, bool)+0xb0d>
  464738:	c1 e7 07             	shl    $0x7,%edi
  46473b:	48 63 c7             	movslq %edi,%rax
  46473e:	62 51 7c 48 11 74 80 	vmovups %zmm14,0x80(%r8,%rax,4)
  464745:	02 
  464746:	62 51 7c 48 11 7c 80 	vmovups %zmm15,0x40(%r8,%rax,4)
  46474d:	01 
  46474e:	62 51 7c 48 11 2c 80 	vmovups %zmm13,(%r8,%rax,4)
  464755:	62 51 7c 48 11 64 80 	vmovups %zmm12,0xc0(%r8,%rax,4)
  46475c:	03 
  46475d:	62 51 7c 48 11 5c 80 	vmovups %zmm11,0x100(%r8,%rax,4)
  464764:	04 
  464765:	62 51 7c 48 11 54 80 	vmovups %zmm10,0x140(%r8,%rax,4)
  46476c:	05 
  46476d:	62 d1 7c 48 11 7c 80 	vmovups %zmm7,0x180(%r8,%rax,4)
  464774:	06 
  464775:	62 d1 7c 48 11 6c 80 	vmovups %zmm5,0x1c0(%r8,%rax,4)
  46477c:	07 
  46477d:	5b                   	pop    %rbx
  46477e:	41 5c                	pop    %r12
  464780:	41 5d                	pop    %r13
  464782:	41 5e                	pop    %r14
  464784:	41 5f                	pop    %r15
  464786:	5d                   	pop    %rbp
  464787:	c5 f8 77             	vzeroupper 
  46478a:	c3                   	retq   
  46478b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

