0000000000467be0 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)>:
  467be0:	55                   	push   %rbp
  467be1:	41 57                	push   %r15
  467be3:	41 56                	push   %r14
  467be5:	41 55                	push   %r13
  467be7:	41 54                	push   %r12
  467be9:	53                   	push   %rbx
  467bea:	48 89 54 24 d8       	mov    %rdx,-0x28(%rsp)
  467bef:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  467bf4:	0f b6 44 24 50       	movzbl 0x50(%rsp),%eax
  467bf9:	8b 6c 24 48          	mov    0x48(%rsp),%ebp
  467bfd:	8b 54 24 40          	mov    0x40(%rsp),%edx
  467c01:	44 8b 74 24 38       	mov    0x38(%rsp),%r14d
  467c06:	4d 63 d0             	movslq %r8d,%r10
  467c09:	4c 89 54 24 f0       	mov    %r10,-0x10(%rsp)
  467c0e:	46 8b 14 91          	mov    (%rcx,%r10,4),%r10d
  467c12:	41 39 ea             	cmp    %ebp,%r10d
  467c15:	44 0f 43 d5          	cmovae %ebp,%r10d
  467c19:	41 39 d2             	cmp    %edx,%r10d
  467c1c:	41 89 d7             	mov    %edx,%r15d
  467c1f:	44 89 54 24 ec       	mov    %r10d,-0x14(%rsp)
  467c24:	45 0f 42 fa          	cmovb  %r10d,%r15d
  467c28:	45 39 f7             	cmp    %r14d,%r15d
  467c2b:	41 0f 97 c5          	seta   %r13b
  467c2f:	41 89 c3             	mov    %eax,%r11d
  467c32:	45 08 eb             	or     %r13b,%r11b
  467c35:	45 85 c9             	test   %r9d,%r9d
  467c38:	78 78                	js     467cb2 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xd2>
  467c3a:	45 89 cc             	mov    %r9d,%r12d
  467c3d:	42 8b 0c a1          	mov    (%rcx,%r12,4),%ecx
  467c41:	39 e9                	cmp    %ebp,%ecx
  467c43:	0f 42 e9             	cmovb  %ecx,%ebp
  467c46:	39 d5                	cmp    %edx,%ebp
  467c48:	0f 42 d5             	cmovb  %ebp,%edx
  467c4b:	44 39 f2             	cmp    %r14d,%edx
  467c4e:	0f 97 c1             	seta   %cl
  467c51:	08 c1                	or     %al,%cl
  467c53:	45 84 db             	test   %r11b,%r11b
  467c56:	0f 84 ac 00 00 00    	je     467d08 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x128>
  467c5c:	84 c0                	test   %al,%al
  467c5e:	0f 84 b0 00 00 00    	je     467d14 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x134>
  467c64:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  467c69:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  467c6d:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  467c71:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  467c75:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  467c79:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  467c7d:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  467c81:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  467c85:	84 c9                	test   %cl,%cl
  467c87:	0f 85 d8 00 00 00    	jne    467d65 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x185>
  467c8d:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  467c92:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  467c99:	00 
  467c9a:	44 89 e9             	mov    %r13d,%ecx
  467c9d:	31 c0                	xor    %eax,%eax
  467c9f:	41 bb 00 00 00 00    	mov    $0x0,%r11d
  467ca5:	84 c9                	test   %cl,%cl
  467ca7:	0f 85 54 01 00 00    	jne    467e01 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x221>
  467cad:	e9 6b 01 00 00       	jmpq   467e1d <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x23d>
  467cb2:	45 84 db             	test   %r11b,%r11b
  467cb5:	0f 84 9c 09 00 00    	je     468657 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  467cbb:	84 c0                	test   %al,%al
  467cbd:	0f 84 a5 01 00 00    	je     467e68 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x288>
  467cc3:	c4 41 38 57 c0       	vxorps %xmm8,%xmm8,%xmm8
  467cc8:	c5 c0 57 ff          	vxorps %xmm7,%xmm7,%xmm7
  467ccc:	c5 c8 57 f6          	vxorps %xmm6,%xmm6,%xmm6
  467cd0:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
  467cd4:	c5 e0 57 db          	vxorps %xmm3,%xmm3,%xmm3
  467cd8:	c5 e8 57 d2          	vxorps %xmm2,%xmm2,%xmm2
  467cdc:	c5 f0 57 c9          	vxorps %xmm1,%xmm1,%xmm1
  467ce0:	c5 f8 57 c0          	vxorps %xmm0,%xmm0,%xmm0
  467ce4:	45 39 f7             	cmp    %r14d,%r15d
  467ce7:	0f 87 cb 01 00 00    	ja     467eb8 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x2d8>
  467ced:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  467cf2:	31 ed                	xor    %ebp,%ebp
  467cf4:	45 31 db             	xor    %r11d,%r11d
  467cf7:	31 d2                	xor    %edx,%edx
  467cf9:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  467d00:	00 
  467d01:	31 c0                	xor    %eax,%eax
  467d03:	e9 30 01 00 00       	jmpq   467e38 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x258>
  467d08:	84 c9                	test   %cl,%cl
  467d0a:	0f 84 47 09 00 00    	je     468657 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  467d10:	31 c9                	xor    %ecx,%ecx
  467d12:	eb 54                	jmp    467d68 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x188>
  467d14:	45 89 c2             	mov    %r8d,%r10d
  467d17:	41 c1 e2 07          	shl    $0x7,%r10d
  467d1b:	4d 63 d2             	movslq %r10d,%r10
  467d1e:	62 b1 7c 48 10 44 93 	vmovups 0x1c0(%rbx,%r10,4),%zmm0
  467d25:	07 
  467d26:	62 b1 7c 48 10 4c 93 	vmovups 0x180(%rbx,%r10,4),%zmm1
  467d2d:	06 
  467d2e:	62 b1 7c 48 10 54 93 	vmovups 0x140(%rbx,%r10,4),%zmm2
  467d35:	05 
  467d36:	62 b1 7c 48 10 5c 93 	vmovups 0x100(%rbx,%r10,4),%zmm3
  467d3d:	04 
  467d3e:	62 31 7c 48 10 04 93 	vmovups (%rbx,%r10,4),%zmm8
  467d45:	62 b1 7c 48 10 7c 93 	vmovups 0x40(%rbx,%r10,4),%zmm7
  467d4c:	01 
  467d4d:	62 b1 7c 48 10 74 93 	vmovups 0x80(%rbx,%r10,4),%zmm6
  467d54:	02 
  467d55:	62 b1 7c 48 10 64 93 	vmovups 0xc0(%rbx,%r10,4),%zmm4
  467d5c:	03 
  467d5d:	84 c9                	test   %cl,%cl
  467d5f:	0f 84 28 ff ff ff    	je     467c8d <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xad>
  467d65:	44 89 e9             	mov    %r13d,%ecx
  467d68:	41 b2 01             	mov    $0x1,%r10b
  467d6b:	44 89 54 24 d4       	mov    %r10d,-0x2c(%rsp)
  467d70:	84 c0                	test   %al,%al
  467d72:	44 88 5c 24 d3       	mov    %r11b,-0x2d(%rsp)
  467d77:	74 35                	je     467dae <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x1ce>
  467d79:	c4 41 10 57 ed       	vxorps %xmm13,%xmm13,%xmm13
  467d7e:	c4 41 00 57 ff       	vxorps %xmm15,%xmm15,%xmm15
  467d83:	c4 41 08 57 f6       	vxorps %xmm14,%xmm14,%xmm14
  467d88:	c4 41 18 57 e4       	vxorps %xmm12,%xmm12,%xmm12
  467d8d:	c4 41 20 57 db       	vxorps %xmm11,%xmm11,%xmm11
  467d92:	c4 41 28 57 d2       	vxorps %xmm10,%xmm10,%xmm10
  467d97:	c4 41 30 57 c9       	vxorps %xmm9,%xmm9,%xmm9
  467d9c:	c5 d0 57 ed          	vxorps %xmm5,%xmm5,%xmm5
  467da0:	31 c0                	xor    %eax,%eax
  467da2:	41 bb 00 00 00 00    	mov    $0x0,%r11d
  467da8:	84 c9                	test   %cl,%cl
  467daa:	75 55                	jne    467e01 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x221>
  467dac:	eb 6f                	jmp    467e1d <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x23d>
  467dae:	44 89 c8             	mov    %r9d,%eax
  467db1:	c1 e0 07             	shl    $0x7,%eax
  467db4:	48 98                	cltq   
  467db6:	62 f1 7c 48 10 6c 83 	vmovups 0x1c0(%rbx,%rax,4),%zmm5
  467dbd:	07 
  467dbe:	62 71 7c 48 10 4c 83 	vmovups 0x180(%rbx,%rax,4),%zmm9
  467dc5:	06 
  467dc6:	62 71 7c 48 10 54 83 	vmovups 0x140(%rbx,%rax,4),%zmm10
  467dcd:	05 
  467dce:	62 71 7c 48 10 5c 83 	vmovups 0x100(%rbx,%rax,4),%zmm11
  467dd5:	04 
  467dd6:	62 71 7c 48 10 2c 83 	vmovups (%rbx,%rax,4),%zmm13
  467ddd:	62 71 7c 48 10 7c 83 	vmovups 0x40(%rbx,%rax,4),%zmm15
  467de4:	01 
  467de5:	62 71 7c 48 10 74 83 	vmovups 0x80(%rbx,%rax,4),%zmm14
  467dec:	02 
  467ded:	62 71 7c 48 10 64 83 	vmovups 0xc0(%rbx,%rax,4),%zmm12
  467df4:	03 
  467df5:	31 c0                	xor    %eax,%eax
  467df7:	41 bb 00 00 00 00    	mov    $0x0,%r11d
  467dfd:	84 c9                	test   %cl,%cl
  467dff:	74 1c                	je     467e1d <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x23d>
  467e01:	48 8b 4c 24 d8       	mov    -0x28(%rsp),%rcx
  467e06:	4c 8b 54 24 f0       	mov    -0x10(%rsp),%r10
  467e0b:	42 8b 0c 91          	mov    (%rcx,%r10,4),%ecx
  467e0f:	45 89 f2             	mov    %r14d,%r10d
  467e12:	49 01 ca             	add    %rcx,%r10
  467e15:	48 8b 4f 18          	mov    0x18(%rdi),%rcx
  467e19:	46 8b 1c 91          	mov    (%rcx,%r10,4),%r11d
  467e1d:	44 39 f2             	cmp    %r14d,%edx
  467e20:	76 16                	jbe    467e38 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x258>
  467e22:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  467e27:	42 8b 04 a0          	mov    (%rax,%r12,4),%eax
  467e2b:	44 89 f1             	mov    %r14d,%ecx
  467e2e:	48 01 c1             	add    %rax,%rcx
  467e31:	48 8b 47 18          	mov    0x18(%rdi),%rax
  467e35:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  467e38:	45 85 c9             	test   %r9d,%r9d
  467e3b:	0f 99 c1             	setns  %cl
  467e3e:	44 39 f2             	cmp    %r14d,%edx
  467e41:	41 0f 97 c2          	seta   %r10b
  467e45:	41 20 ca             	and    %cl,%r10b
  467e48:	45 39 f7             	cmp    %r14d,%r15d
  467e4b:	77 09                	ja     467e56 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x276>
  467e4d:	45 84 d2             	test   %r10b,%r10b
  467e50:	0f 84 b4 03 00 00    	je     46820a <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x62a>
  467e56:	44 89 44 24 e8       	mov    %r8d,-0x18(%rsp)
  467e5b:	89 6c 24 e4          	mov    %ebp,-0x1c(%rsp)
  467e5f:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  467e63:	e9 8c 00 00 00       	jmpq   467ef4 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x314>
  467e68:	44 89 c0             	mov    %r8d,%eax
  467e6b:	c1 e0 07             	shl    $0x7,%eax
  467e6e:	48 98                	cltq   
  467e70:	62 f1 7c 48 10 44 83 	vmovups 0x1c0(%rbx,%rax,4),%zmm0
  467e77:	07 
  467e78:	62 f1 7c 48 10 4c 83 	vmovups 0x180(%rbx,%rax,4),%zmm1
  467e7f:	06 
  467e80:	62 f1 7c 48 10 54 83 	vmovups 0x140(%rbx,%rax,4),%zmm2
  467e87:	05 
  467e88:	62 f1 7c 48 10 5c 83 	vmovups 0x100(%rbx,%rax,4),%zmm3
  467e8f:	04 
  467e90:	62 71 7c 48 10 04 83 	vmovups (%rbx,%rax,4),%zmm8
  467e97:	62 f1 7c 48 10 7c 83 	vmovups 0x40(%rbx,%rax,4),%zmm7
  467e9e:	01 
  467e9f:	62 f1 7c 48 10 74 83 	vmovups 0x80(%rbx,%rax,4),%zmm6
  467ea6:	02 
  467ea7:	62 f1 7c 48 10 64 83 	vmovups 0xc0(%rbx,%rax,4),%zmm4
  467eae:	03 
  467eaf:	45 39 f7             	cmp    %r14d,%r15d
  467eb2:	0f 86 35 fe ff ff    	jbe    467ced <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x10d>
  467eb8:	44 89 44 24 e8       	mov    %r8d,-0x18(%rsp)
  467ebd:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  467ec2:	48 8b 4c 24 f0       	mov    -0x10(%rsp),%rcx
  467ec7:	8b 04 88             	mov    (%rax,%rcx,4),%eax
  467eca:	44 89 f1             	mov    %r14d,%ecx
  467ecd:	48 01 c1             	add    %rax,%rcx
  467ed0:	48 8b 7f 18          	mov    0x18(%rdi),%rdi
  467ed4:	44 8b 1c 8f          	mov    (%rdi,%rcx,4),%r11d
  467ed8:	c6 44 24 d3 01       	movb   $0x1,-0x2d(%rsp)
  467edd:	45 31 d2             	xor    %r10d,%r10d
  467ee0:	31 c0                	xor    %eax,%eax
  467ee2:	c7 44 24 d4 00 00 00 	movl   $0x0,-0x2c(%rsp)
  467ee9:	00 
  467eea:	31 d2                	xor    %edx,%edx
  467eec:	c7 44 24 e4 00 00 00 	movl   $0x0,-0x1c(%rsp)
  467ef3:	00 
  467ef4:	49 63 c9             	movslq %r9d,%rcx
  467ef7:	48 89 4c 24 f8       	mov    %rcx,-0x8(%rsp)
  467efc:	45 89 f4             	mov    %r14d,%r12d
  467eff:	44 89 c9             	mov    %r9d,%ecx
  467f02:	66 66 66 66 66 2e 0f 	data16 data16 data16 data16 nopw %cs:0x0(%rax,%rax,1)
  467f09:	1f 84 00 00 00 00 00 
  467f10:	41 f6 c5 01          	test   $0x1,%r13b
  467f14:	74 3a                	je     467f50 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x370>
  467f16:	44 39 d8             	cmp    %r11d,%eax
  467f19:	41 bd 03 00 00 00    	mov    $0x3,%r13d
  467f1f:	41 83 dd 00          	sbb    $0x0,%r13d
  467f23:	41 f6 c2 01          	test   $0x1,%r10b
  467f27:	41 b8 01 00 00 00    	mov    $0x1,%r8d
  467f2d:	45 0f 44 e8          	cmove  %r8d,%r13d
  467f31:	41 39 c3             	cmp    %eax,%r11d
  467f34:	45 0f 42 e8          	cmovb  %r8d,%r13d
  467f38:	45 8d 56 01          	lea    0x1(%r14),%r10d
  467f3c:	41 f6 c5 01          	test   $0x1,%r13b
  467f40:	75 23                	jne    467f65 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x385>
  467f42:	eb 65                	jmp    467fa9 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3c9>
  467f44:	66 66 66 2e 0f 1f 84 	data16 data16 nopw %cs:0x0(%rax,%rax,1)
  467f4b:	00 00 00 00 00 
  467f50:	45 0f b6 ea          	movzbl %r10b,%r13d
  467f54:	41 83 e5 01          	and    $0x1,%r13d
  467f58:	45 01 ed             	add    %r13d,%r13d
  467f5b:	45 8d 56 01          	lea    0x1(%r14),%r10d
  467f5f:	41 f6 c5 01          	test   $0x1,%r13b
  467f63:	74 44                	je     467fa9 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3c9>
  467f65:	44 3b 54 24 ec       	cmp    -0x14(%rsp),%r10d
  467f6a:	73 3d                	jae    467fa9 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x3c9>
  467f6c:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  467f71:	4c 8b 4c 24 f0       	mov    -0x10(%rsp),%r9
  467f76:	47 8b 0c 88          	mov    (%r8,%r9,4),%r9d
  467f7a:	44 89 f3             	mov    %r14d,%ebx
  467f7d:	4c 01 cb             	add    %r9,%rbx
  467f80:	44 8b 44 9f 04       	mov    0x4(%rdi,%rbx,4),%r8d
  467f85:	41 89 c9             	mov    %ecx,%r9d
  467f88:	49 c1 e0 08          	shl    $0x8,%r8
  467f8c:	42 0f 18 0c 06       	prefetcht0 (%rsi,%r8,1)
  467f91:	42 0f 18 4c 06 40    	prefetcht0 0x40(%rsi,%r8,1)
  467f97:	42 0f 18 8c 06 80 00 	prefetcht0 0x80(%rsi,%r8,1)
  467f9e:	00 00 
  467fa0:	42 0f 18 8c 06 c0 00 	prefetcht0 0xc0(%rsi,%r8,1)
  467fa7:	00 00 
  467fa9:	41 8d 6c 24 01       	lea    0x1(%r12),%ebp
  467fae:	41 f6 c5 02          	test   $0x2,%r13b
  467fb2:	74 43                	je     467ff7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  467fb4:	3b 6c 24 e4          	cmp    -0x1c(%rsp),%ebp
  467fb8:	73 3d                	jae    467ff7 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x417>
  467fba:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  467fbf:	4c 8b 4c 24 f8       	mov    -0x8(%rsp),%r9
  467fc4:	43 8b 1c 88          	mov    (%r8,%r9,4),%ebx
  467fc8:	45 89 e0             	mov    %r12d,%r8d
  467fcb:	49 01 d8             	add    %rbx,%r8
  467fce:	41 89 c9             	mov    %ecx,%r9d
  467fd1:	46 8b 44 87 04       	mov    0x4(%rdi,%r8,4),%r8d
  467fd6:	49 c1 e0 08          	shl    $0x8,%r8
  467fda:	42 0f 18 0c 06       	prefetcht0 (%rsi,%r8,1)
  467fdf:	42 0f 18 4c 06 40    	prefetcht0 0x40(%rsi,%r8,1)
  467fe5:	42 0f 18 8c 06 80 00 	prefetcht0 0x80(%rsi,%r8,1)
  467fec:	00 00 
  467fee:	42 0f 18 8c 06 c0 00 	prefetcht0 0xc0(%rsi,%r8,1)
  467ff5:	00 00 
  467ff7:	41 f6 c5 01          	test   $0x1,%r13b
  467ffb:	75 23                	jne    468020 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x440>
  467ffd:	41 f6 c5 02          	test   $0x2,%r13b
  468001:	0f 85 d1 00 00 00    	jne    4680d8 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x4f8>
  468007:	41 f6 c5 01          	test   $0x1,%r13b
  46800b:	0f 85 7f 01 00 00    	jne    468190 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5b0>
  468011:	41 f6 c5 02          	test   $0x2,%r13b
  468015:	0f 85 9b 01 00 00    	jne    4681b6 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5d6>
  46801b:	e9 b5 01 00 00       	jmpq   4681d5 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5f5>
  468020:	45 89 d8             	mov    %r11d,%r8d
  468023:	49 c1 e0 08          	shl    $0x8,%r8
  468027:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  46802e:	05 
  46802f:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  468036:	04 
  468037:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  46803e:	03 
  46803f:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  468046:	02 
  468047:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  46804e:	01 
  46804f:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  468056:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  46805d:	06 
  46805e:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  468065:	07 
  468066:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  46806d:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  468074:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  46807b:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  468082:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  468089:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  468090:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  468097:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  46809e:	62 b1 6c 48 58 d0    	vaddps %zmm16,%zmm2,%zmm2
  4680a4:	62 b1 64 48 58 d9    	vaddps %zmm17,%zmm3,%zmm3
  4680aa:	62 b1 5c 48 58 e2    	vaddps %zmm18,%zmm4,%zmm4
  4680b0:	62 b1 4c 48 58 f3    	vaddps %zmm19,%zmm6,%zmm6
  4680b6:	62 b1 44 48 58 fc    	vaddps %zmm20,%zmm7,%zmm7
  4680bc:	62 31 3c 48 58 c5    	vaddps %zmm21,%zmm8,%zmm8
  4680c2:	62 b1 74 48 58 ce    	vaddps %zmm22,%zmm1,%zmm1
  4680c8:	62 b1 7c 48 58 c7    	vaddps %zmm23,%zmm0,%zmm0
  4680ce:	41 f6 c5 02          	test   $0x2,%r13b
  4680d2:	0f 84 2f ff ff ff    	je     468007 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x427>
  4680d8:	41 89 c0             	mov    %eax,%r8d
  4680db:	49 c1 e0 08          	shl    $0x8,%r8
  4680df:	62 a2 7d 48 33 44 06 	vpmovzxwd 0xa0(%rsi,%r8,1),%zmm16
  4680e6:	05 
  4680e7:	62 a2 7d 48 33 4c 06 	vpmovzxwd 0x80(%rsi,%r8,1),%zmm17
  4680ee:	04 
  4680ef:	62 a2 7d 48 33 54 06 	vpmovzxwd 0x60(%rsi,%r8,1),%zmm18
  4680f6:	03 
  4680f7:	62 a2 7d 48 33 5c 06 	vpmovzxwd 0x40(%rsi,%r8,1),%zmm19
  4680fe:	02 
  4680ff:	62 a2 7d 48 33 64 06 	vpmovzxwd 0x20(%rsi,%r8,1),%zmm20
  468106:	01 
  468107:	62 a2 7d 48 33 2c 06 	vpmovzxwd (%rsi,%r8,1),%zmm21
  46810e:	62 a2 7d 48 33 74 06 	vpmovzxwd 0xc0(%rsi,%r8,1),%zmm22
  468115:	06 
  468116:	62 a2 7d 48 33 7c 06 	vpmovzxwd 0xe0(%rsi,%r8,1),%zmm23
  46811d:	07 
  46811e:	62 b1 45 40 72 f7 10 	vpslld $0x10,%zmm23,%zmm23
  468125:	62 b1 4d 40 72 f6 10 	vpslld $0x10,%zmm22,%zmm22
  46812c:	62 b1 55 40 72 f5 10 	vpslld $0x10,%zmm21,%zmm21
  468133:	62 b1 5d 40 72 f4 10 	vpslld $0x10,%zmm20,%zmm20
  46813a:	62 b1 65 40 72 f3 10 	vpslld $0x10,%zmm19,%zmm19
  468141:	62 b1 6d 40 72 f2 10 	vpslld $0x10,%zmm18,%zmm18
  468148:	62 b1 75 40 72 f1 10 	vpslld $0x10,%zmm17,%zmm17
  46814f:	62 b1 7d 40 72 f0 10 	vpslld $0x10,%zmm16,%zmm16
  468156:	62 31 2c 48 58 d0    	vaddps %zmm16,%zmm10,%zmm10
  46815c:	62 31 24 48 58 d9    	vaddps %zmm17,%zmm11,%zmm11
  468162:	62 31 1c 48 58 e2    	vaddps %zmm18,%zmm12,%zmm12
  468168:	62 31 0c 48 58 f3    	vaddps %zmm19,%zmm14,%zmm14
  46816e:	62 31 04 48 58 fc    	vaddps %zmm20,%zmm15,%zmm15
  468174:	62 31 14 48 58 ed    	vaddps %zmm21,%zmm13,%zmm13
  46817a:	62 31 34 48 58 ce    	vaddps %zmm22,%zmm9,%zmm9
  468180:	62 b1 54 48 58 ef    	vaddps %zmm23,%zmm5,%zmm5
  468186:	41 f6 c5 01          	test   $0x1,%r13b
  46818a:	0f 84 81 fe ff ff    	je     468011 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x431>
  468190:	45 39 fa             	cmp    %r15d,%r10d
  468193:	73 18                	jae    4681ad <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5cd>
  468195:	4c 8b 44 24 d8       	mov    -0x28(%rsp),%r8
  46819a:	4c 8b 5c 24 f0       	mov    -0x10(%rsp),%r11
  46819f:	47 8b 1c 98          	mov    (%r8,%r11,4),%r11d
  4681a3:	45 89 d6             	mov    %r10d,%r14d
  4681a6:	4d 01 de             	add    %r11,%r14
  4681a9:	46 8b 1c b7          	mov    (%rdi,%r14,4),%r11d
  4681ad:	45 89 d6             	mov    %r10d,%r14d
  4681b0:	41 f6 c5 02          	test   $0x2,%r13b
  4681b4:	74 1f                	je     4681d5 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5f5>
  4681b6:	39 d5                	cmp    %edx,%ebp
  4681b8:	73 18                	jae    4681d2 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x5f2>
  4681ba:	48 8b 44 24 d8       	mov    -0x28(%rsp),%rax
  4681bf:	4c 8b 44 24 f8       	mov    -0x8(%rsp),%r8
  4681c4:	42 8b 04 80          	mov    (%rax,%r8,4),%eax
  4681c8:	41 89 ea             	mov    %ebp,%r10d
  4681cb:	49 01 c2             	add    %rax,%r10
  4681ce:	42 8b 04 97          	mov    (%rdi,%r10,4),%eax
  4681d2:	41 89 ec             	mov    %ebp,%r12d
  4681d5:	45 85 c9             	test   %r9d,%r9d
  4681d8:	41 0f 99 c0          	setns  %r8b
  4681dc:	41 39 d4             	cmp    %edx,%r12d
  4681df:	41 0f 92 c2          	setb   %r10b
  4681e3:	45 20 c2             	and    %r8b,%r10b
  4681e6:	45 39 fe             	cmp    %r15d,%r14d
  4681e9:	41 0f 92 c5          	setb   %r13b
  4681ed:	0f 82 1d fd ff ff    	jb     467f10 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x330>
  4681f3:	45 84 d2             	test   %r10b,%r10b
  4681f6:	0f 85 14 fd ff ff    	jne    467f10 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x330>
  4681fc:	48 8b 5c 24 58       	mov    0x58(%rsp),%rbx
  468201:	8b 6c 24 e4          	mov    -0x1c(%rsp),%ebp
  468205:	44 8b 44 24 e8       	mov    -0x18(%rsp),%r8d
  46820a:	4c 8b 54 24 70       	mov    0x70(%rsp),%r10
  46820f:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  468214:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  468219:	80 7c 24 d3 00       	cmpb   $0x0,-0x2d(%rsp)
  46821e:	0f 84 c6 01 00 00    	je     4683ea <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x80a>
  468224:	41 80 7a 2a 00       	cmpb   $0x0,0x2a(%r10)
  468229:	0f 84 8f 03 00 00    	je     4685be <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9de>
  46822f:	8b 4c 24 ec          	mov    -0x14(%rsp),%ecx
  468233:	3b 4c 24 40          	cmp    0x40(%rsp),%ecx
  468237:	0f 87 81 03 00 00    	ja     4685be <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9de>
  46823d:	62 d1 7d 40 72 d0 10 	vpsrld $0x10,%zmm8,%zmm16
  468244:	41 c1 e0 07          	shl    $0x7,%r8d
  468248:	49 63 f8             	movslq %r8d,%rdi
  46824b:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  468250:	62 e2 7e 48 33 04 7e 	vpmovdw %zmm16,(%rsi,%rdi,2)
  468257:	74 1e                	je     468277 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x697>
  468259:	62 e1 3c 58 54 05 95 	vandps 0x3d895(%rip){1to16},%zmm8,%zmm16        # 4a5af8 <__dso_handle+0x30>
  468260:	d8 03 00 
  468263:	62 31 3c 48 5c c0    	vsubps %zmm16,%zmm8,%zmm8
  468269:	62 d1 3d 48 72 d0 10 	vpsrld $0x10,%zmm8,%zmm8
  468270:	62 72 7e 48 33 04 78 	vpmovdw %zmm8,(%rax,%rdi,2)
  468277:	62 f1 3d 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm8
  46827e:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  468283:	62 72 7e 48 33 44 7e 	vpmovdw %zmm8,0x20(%rsi,%rdi,2)
  46828a:	01 
  46828b:	74 1f                	je     4682ac <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x6cc>
  46828d:	62 71 44 58 54 05 61 	vandps 0x3d861(%rip){1to16},%zmm7,%zmm8        # 4a5af8 <__dso_handle+0x30>
  468294:	d8 03 00 
  468297:	62 d1 44 48 5c f8    	vsubps %zmm8,%zmm7,%zmm7
  46829d:	62 f1 45 48 72 d7 10 	vpsrld $0x10,%zmm7,%zmm7
  4682a4:	62 f2 7e 48 33 7c 78 	vpmovdw %zmm7,0x20(%rax,%rdi,2)
  4682ab:	01 
  4682ac:	62 f1 45 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm7
  4682b3:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  4682b8:	62 f2 7e 48 33 7c 7e 	vpmovdw %zmm7,0x40(%rsi,%rdi,2)
  4682bf:	02 
  4682c0:	74 1f                	je     4682e1 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x701>
  4682c2:	62 f1 4c 58 54 3d 2c 	vandps 0x3d82c(%rip){1to16},%zmm6,%zmm7        # 4a5af8 <__dso_handle+0x30>
  4682c9:	d8 03 00 
  4682cc:	62 f1 4c 48 5c f7    	vsubps %zmm7,%zmm6,%zmm6
  4682d2:	62 f1 4d 48 72 d6 10 	vpsrld $0x10,%zmm6,%zmm6
  4682d9:	62 f2 7e 48 33 74 78 	vpmovdw %zmm6,0x40(%rax,%rdi,2)
  4682e0:	02 
  4682e1:	62 f1 4d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm6
  4682e8:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  4682ed:	62 f2 7e 48 33 74 7e 	vpmovdw %zmm6,0x60(%rsi,%rdi,2)
  4682f4:	03 
  4682f5:	74 1f                	je     468316 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x736>
  4682f7:	62 f1 5c 58 54 35 f7 	vandps 0x3d7f7(%rip){1to16},%zmm4,%zmm6        # 4a5af8 <__dso_handle+0x30>
  4682fe:	d7 03 00 
  468301:	62 f1 5c 48 5c e6    	vsubps %zmm6,%zmm4,%zmm4
  468307:	62 f1 5d 48 72 d4 10 	vpsrld $0x10,%zmm4,%zmm4
  46830e:	62 f2 7e 48 33 64 78 	vpmovdw %zmm4,0x60(%rax,%rdi,2)
  468315:	03 
  468316:	62 f1 5d 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm4
  46831d:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  468322:	62 f2 7e 48 33 64 7e 	vpmovdw %zmm4,0x80(%rsi,%rdi,2)
  468329:	04 
  46832a:	74 1f                	je     46834b <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x76b>
  46832c:	62 f1 64 58 54 25 c2 	vandps 0x3d7c2(%rip){1to16},%zmm3,%zmm4        # 4a5af8 <__dso_handle+0x30>
  468333:	d7 03 00 
  468336:	62 f1 64 48 5c dc    	vsubps %zmm4,%zmm3,%zmm3
  46833c:	62 f1 65 48 72 d3 10 	vpsrld $0x10,%zmm3,%zmm3
  468343:	62 f2 7e 48 33 5c 78 	vpmovdw %zmm3,0x80(%rax,%rdi,2)
  46834a:	04 
  46834b:	62 f1 65 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm3
  468352:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  468357:	62 f2 7e 48 33 5c 7e 	vpmovdw %zmm3,0xa0(%rsi,%rdi,2)
  46835e:	05 
  46835f:	74 1f                	je     468380 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7a0>
  468361:	62 f1 6c 58 54 1d 8d 	vandps 0x3d78d(%rip){1to16},%zmm2,%zmm3        # 4a5af8 <__dso_handle+0x30>
  468368:	d7 03 00 
  46836b:	62 f1 6c 48 5c d3    	vsubps %zmm3,%zmm2,%zmm2
  468371:	62 f1 6d 48 72 d2 10 	vpsrld $0x10,%zmm2,%zmm2
  468378:	62 f2 7e 48 33 54 78 	vpmovdw %zmm2,0xa0(%rax,%rdi,2)
  46837f:	05 
  468380:	62 f1 6d 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm2
  468387:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46838c:	62 f2 7e 48 33 54 7e 	vpmovdw %zmm2,0xc0(%rsi,%rdi,2)
  468393:	06 
  468394:	74 1f                	je     4683b5 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x7d5>
  468396:	62 f1 74 58 54 15 58 	vandps 0x3d758(%rip){1to16},%zmm1,%zmm2        # 4a5af8 <__dso_handle+0x30>
  46839d:	d7 03 00 
  4683a0:	62 f1 74 48 5c ca    	vsubps %zmm2,%zmm1,%zmm1
  4683a6:	62 f1 75 48 72 d1 10 	vpsrld $0x10,%zmm1,%zmm1
  4683ad:	62 f2 7e 48 33 4c 78 	vpmovdw %zmm1,0xc0(%rax,%rdi,2)
  4683b4:	06 
  4683b5:	62 f1 75 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm1
  4683bc:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  4683c1:	62 f2 7e 48 33 4c 7e 	vpmovdw %zmm1,0xe0(%rsi,%rdi,2)
  4683c8:	07 
  4683c9:	74 1f                	je     4683ea <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x80a>
  4683cb:	62 f1 7c 58 54 0d 23 	vandps 0x3d723(%rip){1to16},%zmm0,%zmm1        # 4a5af8 <__dso_handle+0x30>
  4683d2:	d7 03 00 
  4683d5:	62 f1 7c 48 5c c1    	vsubps %zmm1,%zmm0,%zmm0
  4683db:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4683e2:	62 f2 7e 48 33 44 78 	vpmovdw %zmm0,0xe0(%rax,%rdi,2)
  4683e9:	07 
  4683ea:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  4683ef:	0f 84 62 02 00 00    	je     468657 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  4683f5:	41 80 7a 2a 00       	cmpb   $0x0,0x2a(%r10)
  4683fa:	0f 84 11 02 00 00    	je     468611 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa31>
  468400:	39 ea                	cmp    %ebp,%edx
  468402:	0f 85 09 02 00 00    	jne    468611 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa31>
  468408:	62 d1 7d 48 72 d5 10 	vpsrld $0x10,%zmm13,%zmm0
  46840f:	41 c1 e1 07          	shl    $0x7,%r9d
  468413:	49 63 c9             	movslq %r9d,%rcx
  468416:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46841b:	62 f2 7e 48 33 04 4e 	vpmovdw %zmm0,(%rsi,%rcx,2)
  468422:	74 1e                	je     468442 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x862>
  468424:	62 f1 14 58 54 05 ca 	vandps 0x3d6ca(%rip){1to16},%zmm13,%zmm0        # 4a5af8 <__dso_handle+0x30>
  46842b:	d6 03 00 
  46842e:	62 f1 14 48 5c c0    	vsubps %zmm0,%zmm13,%zmm0
  468434:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46843b:	62 f2 7e 48 33 04 48 	vpmovdw %zmm0,(%rax,%rcx,2)
  468442:	62 d1 7d 48 72 d7 10 	vpsrld $0x10,%zmm15,%zmm0
  468449:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46844e:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x20(%rsi,%rcx,2)
  468455:	01 
  468456:	74 1f                	je     468477 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x897>
  468458:	62 f1 04 58 54 05 96 	vandps 0x3d696(%rip){1to16},%zmm15,%zmm0        # 4a5af8 <__dso_handle+0x30>
  46845f:	d6 03 00 
  468462:	62 f1 04 48 5c c0    	vsubps %zmm0,%zmm15,%zmm0
  468468:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46846f:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x20(%rax,%rcx,2)
  468476:	01 
  468477:	62 d1 7d 48 72 d6 10 	vpsrld $0x10,%zmm14,%zmm0
  46847e:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  468483:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x40(%rsi,%rcx,2)
  46848a:	02 
  46848b:	74 1f                	je     4684ac <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x8cc>
  46848d:	62 f1 0c 58 54 05 61 	vandps 0x3d661(%rip){1to16},%zmm14,%zmm0        # 4a5af8 <__dso_handle+0x30>
  468494:	d6 03 00 
  468497:	62 f1 0c 48 5c c0    	vsubps %zmm0,%zmm14,%zmm0
  46849d:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4684a4:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x40(%rax,%rcx,2)
  4684ab:	02 
  4684ac:	62 d1 7d 48 72 d4 10 	vpsrld $0x10,%zmm12,%zmm0
  4684b3:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  4684b8:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x60(%rsi,%rcx,2)
  4684bf:	03 
  4684c0:	74 1f                	je     4684e1 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x901>
  4684c2:	62 f1 1c 58 54 05 2c 	vandps 0x3d62c(%rip){1to16},%zmm12,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4684c9:	d6 03 00 
  4684cc:	62 f1 1c 48 5c c0    	vsubps %zmm0,%zmm12,%zmm0
  4684d2:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4684d9:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x60(%rax,%rcx,2)
  4684e0:	03 
  4684e1:	62 d1 7d 48 72 d3 10 	vpsrld $0x10,%zmm11,%zmm0
  4684e8:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  4684ed:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0x80(%rsi,%rcx,2)
  4684f4:	04 
  4684f5:	74 1f                	je     468516 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x936>
  4684f7:	62 f1 24 58 54 05 f7 	vandps 0x3d5f7(%rip){1to16},%zmm11,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4684fe:	d5 03 00 
  468501:	62 f1 24 48 5c c0    	vsubps %zmm0,%zmm11,%zmm0
  468507:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  46850e:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0x80(%rax,%rcx,2)
  468515:	04 
  468516:	62 d1 7d 48 72 d2 10 	vpsrld $0x10,%zmm10,%zmm0
  46851d:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  468522:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xa0(%rsi,%rcx,2)
  468529:	05 
  46852a:	74 1f                	je     46854b <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x96b>
  46852c:	62 f1 2c 58 54 05 c2 	vandps 0x3d5c2(%rip){1to16},%zmm10,%zmm0        # 4a5af8 <__dso_handle+0x30>
  468533:	d5 03 00 
  468536:	62 f1 2c 48 5c c0    	vsubps %zmm0,%zmm10,%zmm0
  46853c:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  468543:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xa0(%rax,%rcx,2)
  46854a:	05 
  46854b:	62 d1 7d 48 72 d1 10 	vpsrld $0x10,%zmm9,%zmm0
  468552:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  468557:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xc0(%rsi,%rcx,2)
  46855e:	06 
  46855f:	74 1f                	je     468580 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x9a0>
  468561:	62 f1 34 58 54 05 8d 	vandps 0x3d58d(%rip){1to16},%zmm9,%zmm0        # 4a5af8 <__dso_handle+0x30>
  468568:	d5 03 00 
  46856b:	62 f1 34 48 5c c0    	vsubps %zmm0,%zmm9,%zmm0
  468571:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  468578:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xc0(%rax,%rcx,2)
  46857f:	06 
  468580:	62 f1 7d 48 72 d5 10 	vpsrld $0x10,%zmm5,%zmm0
  468587:	41 80 7a 28 00       	cmpb   $0x0,0x28(%r10)
  46858c:	62 f2 7e 48 33 44 4e 	vpmovdw %zmm0,0xe0(%rsi,%rcx,2)
  468593:	07 
  468594:	0f 84 bd 00 00 00    	je     468657 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  46859a:	62 f1 54 58 54 05 54 	vandps 0x3d554(%rip){1to16},%zmm5,%zmm0        # 4a5af8 <__dso_handle+0x30>
  4685a1:	d5 03 00 
  4685a4:	62 f1 54 48 5c c0    	vsubps %zmm0,%zmm5,%zmm0
  4685aa:	62 f1 7d 48 72 d0 10 	vpsrld $0x10,%zmm0,%zmm0
  4685b1:	62 f2 7e 48 33 44 48 	vpmovdw %zmm0,0xe0(%rax,%rcx,2)
  4685b8:	07 
  4685b9:	e9 99 00 00 00       	jmpq   468657 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  4685be:	41 c1 e0 07          	shl    $0x7,%r8d
  4685c2:	49 63 f8             	movslq %r8d,%rdi
  4685c5:	62 f1 7c 48 11 74 bb 	vmovups %zmm6,0x80(%rbx,%rdi,4)
  4685cc:	02 
  4685cd:	62 f1 7c 48 11 7c bb 	vmovups %zmm7,0x40(%rbx,%rdi,4)
  4685d4:	01 
  4685d5:	62 71 7c 48 11 04 bb 	vmovups %zmm8,(%rbx,%rdi,4)
  4685dc:	62 f1 7c 48 11 64 bb 	vmovups %zmm4,0xc0(%rbx,%rdi,4)
  4685e3:	03 
  4685e4:	62 f1 7c 48 11 5c bb 	vmovups %zmm3,0x100(%rbx,%rdi,4)
  4685eb:	04 
  4685ec:	62 f1 7c 48 11 54 bb 	vmovups %zmm2,0x140(%rbx,%rdi,4)
  4685f3:	05 
  4685f4:	62 f1 7c 48 11 4c bb 	vmovups %zmm1,0x180(%rbx,%rdi,4)
  4685fb:	06 
  4685fc:	62 f1 7c 48 11 44 bb 	vmovups %zmm0,0x1c0(%rbx,%rdi,4)
  468603:	07 
  468604:	80 7c 24 d4 00       	cmpb   $0x0,-0x2c(%rsp)
  468609:	0f 85 e6 fd ff ff    	jne    4683f5 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0x815>
  46860f:	eb 46                	jmp    468657 <void gcn_extra_pair_plan::dynamic_pair_window<false, false>(gcn_extra_paper::SourceGraphView const&, unsigned short const*, unsigned int const*, unsigned int const*, int, int, unsigned int, unsigned int, unsigned int, bool, float*, unsigned short*, unsigned short*, gcn_extra_state::Method const&, gcn_extra_pair_plan::Profile&, bool)+0xa77>
  468611:	41 c1 e1 07          	shl    $0x7,%r9d
  468615:	49 63 c1             	movslq %r9d,%rax
  468618:	62 71 7c 48 11 74 83 	vmovups %zmm14,0x80(%rbx,%rax,4)
  46861f:	02 
  468620:	62 71 7c 48 11 7c 83 	vmovups %zmm15,0x40(%rbx,%rax,4)
  468627:	01 
  468628:	62 71 7c 48 11 2c 83 	vmovups %zmm13,(%rbx,%rax,4)
  46862f:	62 71 7c 48 11 64 83 	vmovups %zmm12,0xc0(%rbx,%rax,4)
  468636:	03 
  468637:	62 71 7c 48 11 5c 83 	vmovups %zmm11,0x100(%rbx,%rax,4)
  46863e:	04 
  46863f:	62 71 7c 48 11 54 83 	vmovups %zmm10,0x140(%rbx,%rax,4)
  468646:	05 
  468647:	62 71 7c 48 11 4c 83 	vmovups %zmm9,0x180(%rbx,%rax,4)
  46864e:	06 
  46864f:	62 f1 7c 48 11 6c 83 	vmovups %zmm5,0x1c0(%rbx,%rax,4)
  468656:	07 
  468657:	5b                   	pop    %rbx
  468658:	41 5c                	pop    %r12
  46865a:	41 5d                	pop    %r13
  46865c:	41 5e                	pop    %r14
  46865e:	41 5f                	pop    %r15
  468660:	5d                   	pop    %rbp
  468661:	c5 f8 77             	vzeroupper 
  468664:	c3                   	retq   
  468665:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
  46866c:	00 00 00 
  46866f:	90                   	nop

