# Testing de Instrucciones RTM32 (STX4)

Documentación de pruebas realizadas sobre el procesador STX4 utilizando el debugger interactivo del simulador `rtm32`, conectado vía `telnet localhost 4444` luego de ejecutar `rtm32 -d telnet`.

**Herramientas utilizadas:**
- `rtm32` v0.4 — simulador del procesador STX4
- Debugger interactivo vía telnet (puerto 4444)
- Script Python `rtm32_encoder.py` para codificar instrucciones a hexadecimal

**Convención de registros (manual RTM32):**
- R0=$zero, R1=$at, R2-R3=$k0-$k1, R4-R7=$a0-$a3
- R8-R9=$v0-$v1, R10-R19=$t0-$t9, R20-R27=$s0-$s7
- R28=$fp, R29=$gp, R30=$sp, R31=$ra

---

## Resumen de resultados

| Instruccion | Tipo | Estado | Observacion |
|-------------|------|--------|-------------|
| ADD | R | Funciona | |
| SUB | R | Funciona | |
| AND | R | Funciona | |
| OR | R | Funciona | |
| XOR | R | Funciona | |
| NOR | R | Funciona | |
| SLT | R | Funciona | |
| SLTU | R | Funciona | |
| MUL | R | Funciona | |
| MULH | R | Funciona | |
| DIV | R | Funciona | |
| REST | R | Funciona | |
| SLL | R | Funciona | |
| SRL | R | Funciona | |
| SRA | R | Funciona | |
| SLLR | R | Funciona | |
| SRLR | R | Funciona | |
| SRAR | R | Funciona | |
| JR | R | Funciona | |
| J | J | Funciona | |
| JAL | J | Funciona | |
| ANDI | L | Funciona | |
| ORI | L | Funciona | |
| XORI | L | Funciona | |
| LW | I | Funciona | |
| SW | I | Funciona | Bug de Size=2 en el log corregido en version posterior |
| SH | I | Funciona | |
| SB | I | Funciona | |
| LH | I | Funciona | |
| LB | I | Funciona | |
| LBU | I | Funciona | |
| BEQ | I | Funciona | |
| BNE | I | Funciona | |
| BLT | I | Funciona | |
| BGT | I | Funciona | |
| BLE | I | Funciona | |
| BGE | I | Funciona | |
| SLTI | I | Funciona | |
| SLTIU | I | Funciona | |
| LWX | R | Bug parcial | Resultado va a rt en vez de rd |
| LHX | R | Bug parcial | Resultado va a rt en vez de rd |
| LHUX | R | Bug parcial | Resultado va a rt en vez de rd |
| LBX | R | Bug parcial | Resultado va a rt en vez de rd |
| LBUX | R | Bug parcial | Resultado va a rt en vez de rd |
| JALR | R | Bug parcial | Link siempre va a R0 en vez de rt |
| LHU | I | Funciona | Bug en v0.4, corregido en version posterior |
| CFS | R | No implementada | Ejecuta sin excepcion pero no hace nada |
| CTS | R | No implementada | Ejecuta sin excepcion pero no hace nada |
| ADDI | I | Funciona | Bug en v0.4, corregido en version posterior |
| LUI | L | Funciona | Bug en v0.4, corregido en version posterior |
| MULHU | R | Bug | CAUSE=3, no ejecuta |
| DIVU | R | Bug | CAUSE=3, no ejecuta |
| RESTU | R | Bug | CAUSE=3, no ejecuta |
| RFT | R | Incierto | Ejecuta pero salta a direccion inesperada |
| TRAP | R | Incierto | Dependencia de CTS para verificacion completa |

---

# Caso 1 — ADD

## Descripcion
Testeo de ADD (tipo R), verificando que la suma de dos registros se almacene correctamente en el registro destino y que el PC avance 4 bytes.

## Instrucciones
- ADD

## Precondiciones
- R2 = 0x0000000A (10 decimal)
- R3 = 0x00000005 (5 decimal)
- ADD $1, $2, $3 codificado como 0x0086101C inyectado en [0x0]
- PC en 0x00000000

## Code
```
set r2 0x0000000A
set r3 0x00000005
set [0x0] 0x0086101C
step 1
registers
```
Codificacion (formato R): opcode=00000, rs=2, rt=3, rd=1, aux=0, func=011100 -> 0x0086101C

## Postcondiciones
- R[1] = 0x0000000F (15). Esperado: 10 + 5 = 15. Correcto.
- PC: 0x00000000 -> 0x00000004. Correcto.

## Conclusiones
Anduve. ADD funciona correctamente (R[rd] = R[rs] + R[rt]).

---

# Caso 2 — ANDI

## Descripcion
Testeo de ANDI (tipo L) investigando el bug documentado en el manual (nota al pie 2). Se realizaron tres sub-pruebas para verificar el comportamiento del campo h (mitad baja y mitad alta de la palabra).

## Instrucciones
- ANDI

## Precondiciones
- Sub-test 1: R11 = 0x000000FF, h=0, mascara 0x0F
- Sub-test 2: R11 = 0x000000FF, h=1, mascara 0xFF
- Sub-test 3: R11 = 0x00FF00FF, h=1, mascara 0xFF (prueba decisiva con bits en ambas mitades)

## Code
```
# Sub-test 1: h=0
set r11 0x000000FF
set [0x4] 0x22D4000F
step 1
registers

# Sub-test 2: h=1, valor sin bits altos
set [0x8] 0x22D900FF
step 1
registers

# Sub-test 3: h=1, decisivo
set r11 0x00FF00FF
set [0xC] 0x22DB00FF
step 1
registers
```

## Postcondiciones
- Sub-test 1: R[10] = 0x0000000F. Esperado: 0xFF & 0x0F = 0x0F. Correcto.
- Sub-test 2: R[12] = 0x00000000. Esperado: 0xFF & 0x00FF0000 = 0x00000000. Correcto.
- Sub-test 3: R[13] = 0x00FF0000. Esperado: 0x00FF00FF & 0x00FF0000 = 0x00FF0000. Correcto.

## Conclusiones
Anduve. No se reprodujo el bug documentado en el manual. Los tres sub-tests coinciden con la especificacion. Se re-valido en binario actualizado con resultados identicos. El bug podria requerir condiciones mas especificas no cubiertas en estas pruebas.

---

# Caso 3 — SUB

## Descripcion
Testeo de SUB (tipo R), verificando que la resta se almacene correctamente en el registro destino.

## Instrucciones
- SUB

## Precondiciones
- R2 = 0x0000000F (15)
- R3 = 0x00000005 (5)
- SUB $1, $2, $3 codificado como 0x00861C1D en [0x0]

## Code
```
set r2 0x0000000F
set r3 0x00000005
set [0x0] 0x00861C1D
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x0000000A (10). Esperado: 15 - 5 = 10. Correcto.
- PC: 0x0 -> 0x4. Correcto.

## Conclusiones
Anduve. SUB funciona correctamente (R[rd] = R[rs] - R[rt]).

---

# Caso 4 — OR

## Descripcion
Testeo de OR (tipo R), verificando la operacion logica OR bit a bit entre dos registros.

## Instrucciones
- OR

## Precondiciones
- R2 = 0x0000000F
- R3 = 0x000000F0
- OR $1, $2, $3 codificado como 0x00861009 en [0x0]

## Code
```
set r2 0x0000000F
set r3 0x000000F0
set [0x0] 0x00861009
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x000000FF. Esperado: 0x0F | 0xF0 = 0xFF. Correcto.

## Conclusiones
Anduve. OR funciona correctamente (R[rd] = R[rs] | R[rt]).

---

# Caso 5 — AND

## Descripcion
Testeo de AND (tipo R), verificando la operacion logica AND bit a bit entre dos registros.

## Instrucciones
- AND

## Precondiciones
- R2 = 0x000000FF
- R3 = 0x0000000F
- AND $1, $2, $3 codificado como 0x00861008 en [0x0]

## Code
```
set r2 0x000000FF
set r3 0x0000000F
set [0x0] 0x00861008
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x0000000F. Esperado: 0xFF & 0x0F = 0x0F. Correcto.

## Conclusiones
Anduve. AND funciona correctamente (R[rd] = R[rs] & R[rt]).

---

# Caso 6 — XOR

## Descripcion
Testeo de XOR (tipo R), verificando la operacion logica XOR bit a bit entre dos registros.

## Instrucciones
- XOR

## Precondiciones
- R2 = 0x000000FF
- R3 = 0x0000000F
- XOR $1, $2, $3 codificado como 0x0086100A en [0x0]

## Code
```
set r2 0x000000FF
set r3 0x0000000F
set [0x0] 0x0086100A
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x000000F0. Esperado: 0xFF ^ 0x0F = 0xF0. Correcto.

## Conclusiones
Anduve. XOR funciona correctamente (R[rd] = R[rs] XOR R[rt]).

---

# Caso 7 — NOR

## Descripcion
Testeo de NOR (tipo R), verificando la negacion del OR logico bit a bit entre dos registros.

## Instrucciones
- NOR

## Precondiciones
- R2 = 0x000000FF
- R3 = 0x0000000F
- NOR $1, $2, $3 codificado como 0x0086100B en [0x0]

## Code
```
set r2 0x000000FF
set r3 0x0000000F
set [0x0] 0x0086100B
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0xFFFFFF00. Esperado: ~(0xFF | 0x0F) = 0xFFFFFF00. Correcto.

## Conclusiones
Anduve. NOR funciona correctamente (R[rd] = ~(R[rs] | R[rt])).

---

# Caso 8 — SLT

## Descripcion
Testeo de SLT (set less than, tipo R) verificando el resultado en dos sub-casos: condicion verdadera y condicion falsa.

## Instrucciones
- SLT

## Precondiciones
- Sub-test 1: R2 = 5, R3 = 15 (5 < 15, verdadero)
- Sub-test 2: R2 = 15, R3 = 5 (15 < 5, falso)
- SLT $1, $2, $3 codificado como 0x0086100C

## Code
```
# Sub-test 1: verdadero
set r2 0x00000005
set r3 0x0000000F
set [0x0] 0x0086100C
set pc 0x0
step 1
registers

# Sub-test 2: falso
set r2 0x0000000F
set r3 0x00000005
set pc 0x0
step 1
registers
```

## Postcondiciones
- Sub-test 1: R[1] = 0x00000001. Esperado: 5 < 15 -> 1. Correcto.
- Sub-test 2: R[1] = 0x00000000. Esperado: 15 < 5 -> 0. Correcto.

## Conclusiones
Anduve. SLT funciona correctamente en ambos casos (R[rd] = (R[rs] < R[rt]) ? 1 : 0).

---

# Caso 9 — SLTU

## Descripcion
Testeo de SLTU (set less than unsigned, tipo R). Se usa 0xFFFFFFFF que con signo es -1 pero sin signo es el maximo valor positivo, para distinguirlo de SLT.

## Instrucciones
- SLTU

## Precondiciones
- R2 = 0xFFFFFFFF (sin signo: 4294967295)
- R3 = 0x00000001
- SLTU $1, $2, $3 codificado como 0x0086100D

## Code
```
set r2 0xFFFFFFFF
set r3 0x00000001
set [0x0] 0x0086100D
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x00000000. Esperado sin signo: 4294967295 < 1 -> 0. Correcto.
- Con SLT daria 1 (con signo: -1 < 1), lo que confirma que SLTU trata los valores como unsigned.

## Conclusiones
Anduve. SLTU funciona correctamente y se distingue de SLT en el tratamiento de valores negativos.

---

# Caso 10 — MUL

## Descripcion
Testeo de MUL (multiplicacion con signo, tipo R), que retorna los 32 bits bajos del resultado.

## Instrucciones
- MUL

## Precondiciones
- R2 = 0x00000006 (6)
- R3 = 0x00000007 (7)
- MUL $1, $2, $3 codificado como 0x00861015

## Code
```
set r2 0x00000006
set r3 0x00000007
set [0x0] 0x00861015
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x0000002A (42). Esperado: 6 x 7 = 42. Correcto.

## Conclusiones
Anduve. MUL funciona correctamente (R[rd] = (R[rs] x R[rt])[31:0]).

---

# Caso 11 — MULH

## Descripcion
Testeo de MULH (parte alta de multiplicacion con signo, tipo R). Se usa un valor grande para generar bits en la parte alta del resultado de 64 bits.

## Instrucciones
- MULH

## Precondiciones
- R2 = 0x07FFFFFF
- R3 = 0x07FFFFFF
- MULH $1, $2, $3 codificado como 0x00861016

## Code
```
set r2 0x07FFFFFF
set r3 0x07FFFFFF
set [0x0] 0x00861016
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x003FFFFF. Esperado: parte alta de 0x07FFFFFF x 0x07FFFFFF = 0x003FFFFF_F0000001. Correcto.

## Conclusiones
Anduve. MULH retorna correctamente los 32 bits altos del producto con signo.

---

# Caso 12 — DIV

## Descripcion
Testeo de DIV (division entera con signo, tipo R).

## Instrucciones
- DIV

## Precondiciones
- R2 = 0x0000001E (30)
- R3 = 0x00000006 (6)
- DIV $1, $2, $3 codificado como 0x00861018

## Code
```
set r2 0x0000001E
set r3 0x00000006
set [0x0] 0x00861018
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x00000005 (5). Esperado: 30 / 6 = 5. Correcto.

## Conclusiones
Anduve. DIV funciona correctamente (R[rd] = R[rs] / R[rt]).

---

# Caso 13 — REST

## Descripcion
Testeo de REST (resto de division entera con signo, tipo R).

## Instrucciones
- REST

## Precondiciones
- R2 = 0x0000001D (29)
- R3 = 0x00000006 (6)
- REST $1, $2, $3 codificado como 0x0086101A

## Code
```
set r2 0x0000001D
set r3 0x00000006
set [0x0] 0x0086101A
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x00000005 (5). Esperado: 29 % 6 = 5. Correcto.

## Conclusiones
Anduve. REST funciona correctamente (R[rd] = R[rs] % R[rt]).

---

# Caso 14 — ORI

## Descripcion
Testeo de ORI (OR inmediato, tipo L), verificando la operacion OR entre un registro y una constante.

## Instrucciones
- ORI

## Precondiciones
- R2 = 0x000000F0
- ORI $1, $2, 0x0F codificado como 0x2882000F

## Code
```
set r2 0x000000F0
set [0x0] 0x2882000F
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x000000FF. Esperado: 0xF0 | 0x0F = 0xFF. Correcto.

## Conclusiones
Anduve. ORI funciona correctamente (R[rt] = R[rs] | ZE(ims)).

---

# Caso 15 — XORI

## Descripcion
Testeo de XORI (XOR inmediato, tipo L).

## Instrucciones
- XORI

## Precondiciones
- R2 = 0x000000F0
- XORI $1, $2, 0xFF codificado como 0x308200FF

## Code
```
set r2 0x000000F0
set [0x0] 0x308200FF
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x0000000F. Esperado: 0xF0 ^ 0xFF = 0x0F. Correcto.

## Conclusiones
Anduve. XORI funciona correctamente (R[rt] = R[rs] XOR ZE(ims)).

---

# Caso 16 — JR

## Descripcion
Testeo de JR (jump register, tipo R), verificando que el PC tome el valor del registro fuente.

## Instrucciones
- JR

## Precondiciones
- R2 = 0x00000010 (direccion de salto)
- JR $2 codificado como 0x0080000E en [0x0]

## Code
```
set r2 0x00000010
set [0x0] 0x0080000E
set pc 0x0
step 1
registers
```

## Postcondiciones
- PC = 0x00000010. Esperado: PC toma el valor de R2. Correcto.

## Conclusiones
Anduve. JR funciona correctamente (PC = R[rs]).

---

# Caso 17 — J

## Descripcion
Testeo de J (jump incondicional, tipo J), verificando el salto a una direccion absoluta codificada en la instruccion.

## Instrucciones
- J

## Precondiciones
- J 4 codificado como 0x10000004 en [0x0]
- La direccion en palabras es 4, equivalente a byte address 0x10

## Code
```
set [0x0] 0x10000004
set pc 0x0
step 1
registers
```

## Postcondiciones
- PC = 0x00000010. Esperado: salto a direccion 4 x 4 = 0x10. Correcto.

## Conclusiones
Anduve. J funciona correctamente (PC = E(address)).

---

# Caso 18 — JAL

## Descripcion
Testeo de JAL (jump and link, tipo J), verificando que salte correctamente y guarde la direccion de retorno en R[31].

## Instrucciones
- JAL

## Precondiciones
- JAL 4 codificado como 0x18000004 en [0x0]
- PC en 0x0, por lo tanto PC+4 = 0x4

## Code
```
set [0x0] 0x18000004
set pc 0x0
step 1
registers
```

## Postcondiciones
- PC = 0x00000010. Correcto.
- R[31] = 0x00000004. Esperado: PC + 4 = 0x4. Correcto.

## Conclusiones
Anduve. JAL funciona correctamente (R[31] = PC+4; PC = E(address)).

---

# Caso 19 — SLL, SRL, SRA, SLLR, SRLR, SRAR

## Descripcion
Testeo del grupo completo de instrucciones de desplazamiento (tipo R). Se prueban los shifts por constante (SLL, SRL, SRA) y por registro (SLLR, SRLR, SRAR) en una sola secuencia cargada en memoria. Nota: en una sesion anterior SLL habia fallado con CAUSE=3, pero se determino que fue por estado corrupto de la maquina tras excepciones previas. Desde estado limpio todos funcionaron correctamente.

## Instrucciones
- SLL, SRL, SRA, SLLR, SRLR, SRAR

## Precondiciones
- R3 = 0x000000F0 (valor a desplazar)
- R2 = 0x00000004 (cantidad de desplazamiento para shifts por registro)
- Instrucciones cargadas secuencialmente desde 0x0

## Code
```
# SRL probado por separado primero
set r3 0x000000F0
set [0x0] 0x00061201
set pc 0x0
step 1
registers

# Secuencia completa SLL, SRA, SLLR, SRLR, SRAR
set r3 0x000000F0
set r2 0x00000004
set [0x0]  0x00061200
set [0x4]  0x00061202
set [0x8]  0x00861003
set [0xC]  0x00861004
set [0x10] 0x00861005
set pc 0x0
step 1
registers
step 1
registers
step 1
registers
step 1
registers
step 1
registers
```

## Postcondiciones
- SRL:  R[1] = 0x0000000F. Esperado: 0xF0 >> 4 = 0xF (logico). Correcto.
- SLL:  R[1] = 0x00000F00. Esperado: 0xF0 << 4 = 0xF00. Correcto.
- SRA:  R[1] = 0x0000000F. Esperado: 0xF0 >> 4 = 0xF (aritmetico, signo=0). Correcto.
- SLLR: R[1] = 0x00000F00. Esperado: 0xF0 << R2(4) = 0xF00. Correcto.
- SRLR: R[1] = 0x0000000F. Esperado: 0xF0 >> R2(4) = 0xF. Correcto.
- SRAR: R[1] = 0x0000000F. Esperado: 0xF0 >> R2(4) = 0xF (aritmetico). Correcto.

## Conclusiones
Anduvieron todos. Los seis shifts funcionan correctamente.

---

# Caso 20 — SW, SH, SB, LW, LH, LB, LBU

## Descripcion
Testeo del grupo de instrucciones de acceso a memoria (tipo I). Se verifica la escritura y lectura de palabras, medias palabras y bytes, incluyendo la extension de signo en LH y LB.

## Instrucciones
- SW, SH, SB, LW, LH, LB, LBU

## Precondiciones
- R1 = 0x000080FF (valor con bit15=1 para probar extension de signo en LH, y bit7=1 para LB)
- R1 = 0x12345678 para la prueba de SW y LW
- Instrucciones inyectadas en [0x0] o [0x4] segun el caso

## Code
```
reset
set r1 0x12345678

# SW: escribe palabra completa en mem[0]
set [0x0] 0x48020000
set pc 0x0
step 1
examine 0x0

# LW: carga la palabra que acaba de escribir
set r1 0x00000000
set [0x4] 0x40020000
set pc 0x4
step 1
registers

# SH: escribe media palabra baja
set r1 0x000080FF
set [0x0] 0x50020000
set pc 0x0
step 1
examine 0x0

# LH: carga media palabra con extension de signo
set r2 0x00000000
set [0x4] 0x60040000
set pc 0x4
step 1
registers

# SB: escribe byte bajo
set [0x0] 0x58020000
set pc 0x0
step 1
examine 0x0

# LB: carga byte con extension de signo
set r2 0x00000000
set [0x4] 0x70040000
set pc 0x4
step 1
registers

# LBU: carga byte sin extension de signo
set r2 0x00000000
set [0x4] 0x78040000
set pc 0x4
step 1
registers
```

## Postcondiciones
- SW: examine 0x0 = 0x12345678. Last Memory Operation Size=2 (bug de logging, dato correcto). Correcto.
- LW: R[1] = 0x12345678. Esperado: carga la palabra completa. Correcto.
- SH: escribe los 16 bits bajos (0x80FF). Size=2. Correcto.
- LH: R[2] = 0xFFFF80FF. Esperado: extension de signo (bit15=1 -> FFFF). Correcto.
- SB: escribe el byte bajo (0xFF). Size=1. Correcto.
- LB: R[2] = 0xFFFFFFFF. Esperado: extension de signo del byte (bit7=1 -> FFFFFF). Correcto.
- LBU: R[2] = 0x000000FF. Esperado: sin extension de signo. Correcto.

## Conclusiones
Anduvieron todas. Las instrucciones de acceso a memoria funcionan correctamente. Se observo que SW reportaba Size=2 en el log del debugger (en vez de Size=4) pero el dato escrito en memoria era correcto. Ver Caso 20b para la re-verificacion en la version corregida.

---

# Caso 20b — SW (re-verificacion, bug de Size corregido)

## Descripcion
Re-testeo de SW luego del comunicado del profesor indicando que se corrigio el bug de logging en la version nueva de la maquina (mismo PDF con reorganizacion de registros que no afecta la codificacion).

## Instrucciones
- SW

## Precondiciones
- R1 = 0x12345678
- SW $1, 0($zero) codificado como 0x48020000 en [0x0]
- PC en 0x0 tras reset

## Code
```
reset
set r1 0x12345678
set [0x0] 0x48020000
set pc 0x0
step 1
examine 0x0
registers
```

## Postcondiciones
- examine 0x0 = 0x12345678. Dato correcto.
- registers -> Last Memory Operation: Address=0x00000000, Size=0x00000004, Type=WRITE. Esperado: Size=4 (palabra completa). Correcto.
- Nota: en esta version, el detalle de "Last Memory Operation" ya no se imprime automaticamente tras `step 1`, hay que consultarlo con `registers`.

## Conclusiones
Bug corregido. SW ahora reporta correctamente Size=0x00000004 en el log del debugger, coincidiendo con el dato escrito en memoria. Confirmado por el profesor en el comunicado de la nueva version de la maquina.

---

# Caso 21 — LHU (bug)

## Descripcion
Testeo de LHU (load halfword unsigned, tipo I). A diferencia de LH que extiende con signo, LHU deberia extender con ceros y leer 2 bytes. Se separo en un caso propio porque presenta un comportamiento incorrecto.

## Instrucciones
- LHU

## Precondiciones
- mem[0x0] = 0x000080FF (tiene bit15=1 para distinguir LHU de LH)
- R1 ya contiene el dato escrito previamente

## Code
```
reset
set r1 0x000080FF
set [0x0] 0x50020000
set pc 0x0
step 1

set r2 0x00000000
set [0x4] 0x68040000
set pc 0x4
step 1
registers
```

## Postcondiciones
- R[2] = 0xFFFFFFFF. Esperado: 0x000080FF (sin extension de signo, 2 bytes).
- Last Memory Operation Size=1 en vez de Size=2.

## Conclusiones
No funciona correctamente. LHU lee 1 byte en vez de 2 y extiende con signo en vez de con ceros, comportandose como LB en vez de como LHU. El resultado es incorrecto tanto en el tamano leido como en la extension aplicada.

---

# Caso 22 — BEQ, BNE, BLT, BGT, BLE, BGE

## Descripcion
Testeo del grupo de instrucciones de salto condicional (tipo I). Se prueba cada branch con una condicion verdadera para verificar que salta, y con una condicion falsa para verificar que no salta.

## Instrucciones
- BEQ, BNE, BLT, BGT, BLE, BGE

## Precondiciones
- R1 = 0x00000005 (5)
- R2 = 0x0000000A (10)
- Offset de salto = 2 instrucciones en todos los casos
- Si la condicion es verdadera: PC esperado = 0xC (salto tomado)
- Si la condicion es falsa: PC esperado = 0x4 (salto no tomado)

## Code
```
reset
set r1 0x00000005
set r2 0x0000000A

# BEQ: R1 == R2 -> falso (5 != 10)
set [0x0] 0x80440002
set pc 0x0
step 1
registers

# BNE: R1 != R2 -> verdadero
set [0x0] 0x88440002
set pc 0x0
step 1
registers

# BLT: R1 < R2 -> verdadero (5 < 10)
set [0x0] 0x90440002
set pc 0x0
step 1
registers

# BGT: R1 > R2 -> falso (5 no es mayor que 10)
set [0x0] 0x98440002
set pc 0x0
step 1
registers

# BLE: R1 <= R2 -> verdadero (5 <= 10)
set [0x0] 0xA0440002
set pc 0x0
step 1
registers

# BGE: R1 >= R2 -> falso (5 no es mayor o igual que 10)
set [0x0] 0xA8440002
set pc 0x0
step 1
registers

# BEQ con iguales: R0 == R0 -> verdadero
set [0x0] 0x80000002
set pc 0x0
step 1
registers
```

## Postcondiciones
- BEQ (5 != 10): PC = 0x4. No salto. Correcto.
- BNE (5 != 10): PC = 0xC. Salto tomado. Correcto.
- BLT (5 < 10): PC = 0xC. Salto tomado. Correcto.
- BGT (5 no > 10): PC = 0x4. No salto. Correcto.
- BLE (5 <= 10): PC = 0xC. Salto tomado. Correcto.
- BGE (5 no >= 10): PC = 0x4. No salto. Correcto.
- BEQ (0 == 0): PC = 0xC. Salto tomado. Correcto.

## Conclusiones
Anduvieron todos. Los seis branches condicionales funcionan correctamente.

---

# Caso 23 — SLTI y SLTIU

## Descripcion
Testeo de SLTI y SLTIU (set less than immediate, tipos I). Se verifica la comparacion con constante inmediata con y sin signo.

## Instrucciones
- SLTI, SLTIU

## Precondiciones
- R1 = 0x00000005 (5)
- Inmediato = 10
- SLTI $2, $1, 10 codificado como 0xB044000A
- SLTIU $2, $1, 10 codificado como 0xB844000A

## Code
```
set r1 0x00000005
set [0x0] 0xB044000A
set pc 0x0
step 1
registers

set [0x0] 0xB844000A
set pc 0x0
step 1
registers
```

## Postcondiciones
- SLTI: R[2] = 0x00000001. Esperado: 5 < 10 -> 1. Correcto.
- SLTIU: R[2] = 0x00000001. Esperado: 5 < 10 (unsigned) -> 1. Correcto.

## Conclusiones
Anduvieron. SLTI y SLTIU funcionan correctamente.

---

---

# Caso 24 — MULHU, DIVU, RESTU (bugs)

## Descripcion
Testeo de las variantes unsigned de multiplicacion, division y modulo (tipo R). Se prueban separadas de sus equivalentes con signo para documentar que generan excepcion mientras MUL, DIV y REST funcionan correctamente.

## Instrucciones
- MULHU, DIVU, RESTU

## Precondiciones
- R2 = 0xFFFFFFFE, R3 = 0x00000002 para MULHU y DIVU
- R2 = 0xFFFFFFFE, R3 = 0x00000003 para RESTU
- MULHU $1, $2, $3 codificado como 0x00861017
- DIVU $1, $2, $3 codificado como 0x00861019
- RESTU $1, $2, $3 codificado como 0x0086101B

## Code
```
set r2 0xFFFFFFFE
set r3 0x00000002
set [0x0] 0x00861017
set pc 0x0
step 1
registers

set [0x0] 0x00861019
set pc 0x0
step 1
registers

set r3 0x00000003
set [0x0] 0x0086101B
set pc 0x0
step 1
registers
```

## Postcondiciones
- MULHU: CAUSE=0x3, PC no avanzo. R[1] sin cambios.
- DIVU:  CAUSE=0x3, PC no avanzo. R[1] sin cambios.
- RESTU: CAUSE=0x3, PC no avanzo. R[1] sin cambios.

## Conclusiones
Las tres instrucciones generan excepcion con CAUSE=3 y no ejecutan. Sus equivalentes con signo (MUL, DIV, REST) funcionan correctamente con los mismos valores de registro, confirmando que el problema es especifico de las variantes unsigned.

---

# Caso 25 — LWX, LHX, LHUX, LBX, LBUX

## Descripcion
Testeo del grupo de instrucciones de carga de memoria indexada (tipo R, funct 16-20). Se verifica el tamano de lectura, la extension de signo y el registro destino efectivo.

## Instrucciones
- LWX, LHX, LHUX, LBX, LBUX

## Precondiciones
- mem[0x0] = 0x0000ABCD
- R2 = 0x00000000, R3 = 0x00000000 (direccion efectiva = 0)
- Instrucciones inyectadas en [0x4] y ejecutadas desde PC=0x4

## Code
```
set [0x0] 0x0000ABCD
set r2 0x00000000
set r3 0x00000000

set [0x4] 0x0086A014
set pc 0x4
step 1
registers

set [0x4] 0x0086A010
set pc 0x4
step 1
registers

set [0x4] 0x0086A011
set pc 0x4
step 1
registers

set [0x4] 0x0086A012
set pc 0x4
step 1
registers

set [0x4] 0x0086A013
set pc 0x4
step 1
registers
```

## Postcondiciones
- LWX:  R[3] = valor con bits altos contaminados. Esperado en R[10]. Destino incorrecto y valor parcialmente incorrecto.
- LHX:  R[3] = 0xFFFFABCD. Esperado en R[10]: 0xFFFFABCD (extension de signo correcta). Destino incorrecto, valor correcto.
- LHUX: R[3] = 0x0000ABCD. Esperado en R[10]: 0x0000ABCD. Destino incorrecto, valor correcto.
- LBX:  R[3] = 0xFFFFFFCD. Esperado en R[10]: 0xFFFFFFCD. Destino incorrecto, valor correcto.
- LBUX: R[3] = 0x000000CD. Esperado en R[10]: 0x000000CD. Destino incorrecto, valor correcto.

## Conclusiones
Funcionan parcialmente. La lectura de memoria es correcta en todas (tamano, extension de signo). El bug es que todas guardan el resultado en rt en vez de rd como especifica el manual. LWX ademas muestra contaminacion en los bits altos.

---

# Caso 26 — JALR

## Descripcion
Testeo de JALR (jump and link register, tipo R). Se verifica el salto y el guardado del link en el registro rt especificado.

## Instrucciones
- JALR

## Precondiciones
- Sub-test 1: JALR $2, $0 — link deberia ir a R0 (hardwired a 0)
- Sub-test 2: JALR $2, $31 — link deberia ir a R31
- R2 = 0x00000010 en ambos casos

## Code
```
set r2 0x00000010
set [0x0] 0x0080000F
set pc 0x0
step 1
registers

set r2 0x00000010
set [0x0] 0x00BE000F
set pc 0x0
step 1
registers
```

## Postcondiciones
- Sub-test 1: PC=0x10 correcto. R[0]=0x00000004, R0 deberia ser hardwired a 0.
- Sub-test 2: PC=0x10 correcto. R[31]=0x00000000, R[0]=0x00000004. Link fue a R0 en vez de R31.

## Conclusiones
El salto funciona pero el link tiene un bug. JALR siempre guarda PC+4 en R[0] ignorando el campo rt, y ademas viola la propiedad hardwired de R[0]=0. A diferencia de JAL que guarda el link correctamente en R[31], JALR no respeta el campo rt.

---

# Caso 27 — CFS y CTS

## Descripcion
Testeo de CFS (copy from special) y CTS (copy to special), que leen y escriben los registros especiales del procesador. Se usa aux=4 para acceder a VBR cuyo valor conocido tras reset es 0xF0000000.

## Instrucciones
- CFS, CTS

## Precondiciones
- VBR = 0xF0000000 (valor conocido tras reset)
- R2 = 0x12345678 (valor a escribir con CTS)
- CFS $1, 4 codificado como 0x00400206
- CTS $2, 4 codificado como 0x00800207

## Code
```
set r2 0x12345678
set [0x0] 0x00800207
set pc 0x0
step 1
registers

set [0x4] 0x00400206
set pc 0x4
step 1
registers
```

## Postcondiciones
- CTS: PC avanzo a 0x4, CAUSE=0. Sin excepcion. VBR sigue en 0xF0000000, no modifico el registro especial.
- CFS: PC avanzo a 0x8, CAUSE=0. Sin excepcion. R[1] = 0x00000000, no copio VBR al registro.

## Conclusiones
No funcionan. Ambas instrucciones se ejecutan sin generar excepcion pero no realizan ninguna operacion. Son instrucciones decodificadas pero sin implementacion.

---

# Caso 28 — RFT

## Descripcion
Testeo de RFT (return from trap, tipo R, funct=33). Deberia copiar EPC al PC. No es posible setear EPC directamente desde el debugger, lo que limita la verificacion.

## Instrucciones
- RFT

## Precondiciones
- RFT codificado como 0x00000021 en [0x0]
- EPC = 0x00000000 (valor tras reset, no modificable desde el debugger)

## Code
```
set [0x0] 0x00000021
set pc 0x0
step 1
registers
```

## Postcondiciones
- PC = 0x00000084. No coincide con EPC = 0x00000000 ni con PC+4 = 0x00000004.
- CAUSE = 0x00000000. No genero excepcion.
- examine 0x00000084 = 0x00000000. La direccion de destino no tiene instrucciones.

## Conclusiones
Comportamiento incierto. RFT ejecuto sin excepcion pero salto a 0x00000084, una direccion que no coincide con ningún valor conocido. No se puede concluir si funciona o no sin poder setear EPC a un valor controlado desde el debugger.

---

# Caso 29 — TRAP

## Descripcion
Testeo de TRAP (tipo R, funct=32). Deberia guardar PC+4 en EPC y saltar a la direccion almacenada en mem[aux*4]. El profe recomendo dejar esta instruccion para el final por su complejidad.

## Instrucciones
- TRAP

## Precondiciones
- mem[0x0] = 0x00000010 (direccion del handler para TRAP 0)
- TRAP 0 codificado como 0x00000020
- TRAP 1 codificado como 0x000000A0

## Code
```
set [0x0] 0x00000010
set [0x4] 0x00000020
set pc 0x4
step 1
registers

set [0x4] 0x00000020
set [0x8] 0x000000A0
set pc 0x8
step 1
registers
```

## Postcondiciones
- Ambos casos: CAUSE=0x3, BADVADR=0x00000004, VBR=0x00000002, PC no avanzo.
- Last Memory Operation: Address=0x00000004, Type=FETCH.

## Conclusiones
Comportamiento incierto. TRAP parece disparar la excepcion correctamente pero el handler no existe en la direccion a la que salta (VBR=0x00000002 es invalida), lo que genera un fault en cascada con CAUSE=3. Para verificarlo completamente se necesitaria setear VBR a una direccion valida usando CTS, pero CTS no esta implementada. La dependencia CTS->VBR->TRAP impide una verificacion conclusiva.

---

---

# Caso 30 — ADDI

## Descripcion
Testeo de ADDI (tipo I), suma de un registro con una constante inmediata. Esta instruccion fallaba con CAUSE=3 en la version v0.4 del simulador. El profe publico una version corregida donde funciona correctamente.

## Instrucciones
- ADDI

## Precondiciones
- ADDI $2, $zero, 42 codificado como 0x0804002A
- PC en 0x0, todos los registros en 0 tras reset

## Code
```
reset
set [0x0] 0x0804002A
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[2] = 0x0000002A (42). Esperado: 0 + 42 = 42. Correcto.
- CAUSE = 0x0. Sin excepcion.
- PC: 0x0 -> 0x4. Correcto.

## Conclusiones
Anduve en la version corregida. ADDI funciona correctamente (R[rt] = R[rs] + SE(imm)).

---

# Caso 31 — LUI

## Descripcion
Testeo de LUI (tipo L), carga una constante de 16 bits en la mitad alta del registro destino dejando los bits bajos en 0. Esta instruccion fallaba con CAUSE=3 en la version v0.4. Corregida en la misma version que ADDI.

## Instrucciones
- LUI

## Precondiciones
- LUI $1, 0x00FF codificado como 0x380200FF

## Code
```
set [0x0] 0x380200FF
set pc 0x0
step 1
registers
```

## Postcondiciones
- R[1] = 0x00FF0000. Esperado: 0x00FF << 16 = 0x00FF0000. Correcto.
- CAUSE = 0x0. Sin excepcion.

## Conclusiones
Anduve en la version corregida. LUI funciona correctamente (R[rt] = ZC(ims)).

---

---

# Caso 32 — LHU

## Descripcion
Testeo de LHU (load halfword unsigned, tipo I). Esta instruccion fallaba en v0.4 leyendo 1 byte con extension de signo en vez de 2 bytes sin extension. Corregida en version posterior.

## Instrucciones
- LHU

## Precondiciones
- mem[0x0] = 0x000080FF (bit15=1 para distinguir de LH que extiende con signo)
- LHU $2, 0($zero) codificado como 0x68040000

## Code
```
reset
set r1 0x000080FF
set [0x0] 0x50020000
set pc 0x0
step 1
set [0x4] 0x68040000
set pc 0x4
step 1
registers
```

## Postcondiciones
- R[2] = 0x000080FF. Esperado: carga 2 bytes sin extension de signo. Correcto.
- Last Memory Operation Size=2. Correcto.
- CAUSE = 0x0. Sin excepcion.

## Conclusiones
Anduve en la version corregida. LHU funciona correctamente (R[rt] = ZE(M[EA][15:0])).

---

# Nota sobre el comando examine

En la version corregida el profe actualizo el comando examine con muchas mas opciones. La sintaxis es:

```
examine [/<format><size>] <address> <count>
```

Opciones de formato: x (hex), t (binario), d (decimal), o (octal), s (string)
Opciones de tamano: w (word/4 bytes), h (halfword/2 bytes), b (byte)

Ejemplos probados con mem[0x10] = 0xDEADBEEF:
```
examine /xw 0x10 1   ->  0xDEADBEEF
examine /xh 0x10 1   ->  0xBEEF
examine /xb 0x10 1   ->  0xEF
examine /tb 0x10 1   ->  0b1110_1111
examine /dw 0x10 1   ->  -559038737
examine /ow 0x10 1   ->  0o33653337357
examine /sw 0x10 1   ->  '.'
examine /xw 0x0  4   ->  muestra 4 palabras consecutivas en una linea
```

El formato binario muestra separadores de nibble (0b1110_1111) lo que facilita la lectura. El formato string muestra '.' para caracteres no imprimibles. Direcciones no alineadas generan error.

---

# Bugs encontrados

## Bug 1 — Variantes unsigned no implementadas

MULHU (funct=23), DIVU (funct=25) y RESTU (funct=27) generan CAUSE=3. Sus equivalentes con signo (MUL, DIV, REST) funcionan correctamente.

```
set [0x0] 0x00861017   # MULHU -> CAUSE=3
set [0x0] 0x00861019   # DIVU  -> CAUSE=3
set [0x0] 0x0086101B   # RESTU -> CAUSE=3
```

## Bug 2 — LWX/LHX/LHUX/LBX/LBUX guardan resultado en rt en vez de rd

Todas las instrucciones de carga indexada usan rt como registro destino en vez de rd como especifica el manual. La lectura de memoria en si (tamano, extension de signo) es correcta.

```
# Con rd=$t0 (R10) y rt=$3 (R3):
# el resultado siempre aparece en R[3], R[10] queda en 0
```

## Bug 3 — JALR guarda link en R0 en vez de rt

JALR ignora el campo rt y siempre guarda PC+4 en R[0], violando ademas la propiedad hardwired de R[0]=0. El salto en si funciona correctamente.

```
# JALR $2, $31 -> PC=0x10 (correcto) pero R[31]=0, R[0]=0x4 (incorrecto)
set [0x0] 0x00BE000F
set pc 0x0
step 1
```

## Bug 4 — CFS y CTS sin implementacion

Ambas instrucciones se ejecutan sin excepcion pero no realizan ninguna operacion. CTS no modifica el registro especial destino y CFS no copia el valor al registro general.

## Bug 5 — LHU lee 1 byte en vez de 2 (corregido en version posterior)

LHU (load halfword unsigned) leia 1 byte en vez de 2 y extendia con signo en vez de con ceros en la version v0.4, comportandose como LB. El profe confirmo que el problema era que el encoding del LHU no estaba implementado y continuaba a una seccion del codigo que no correspondia. Corregido en version posterior.

Prueba en version con bug:
```
# LHU con mem[0] = 0x000080FF
# Resultado v0.4: R[2] = 0xFFFFFFFF, Size=1
```

Prueba en version corregida:
```
reset
set r1 0x000080FF
set [0x0] 0x50020000
set pc 0x0
step 1
set [0x4] 0x68040000
set pc 0x4
step 1
registers
# Resultado: R[2] = 0x000080FF, Size=2. Correcto.
```

## Bug 6 — SW reporta Size=2 en el log del debugger (CORREGIDO)

SW escribia correctamente los 4 bytes en memoria (verificado con examine), pero Last Memory Operation reportaba Size=0x00000002 en vez de Size=0x00000004. Era un bug del logging del debugger, no de la instruccion en si. Corregido por el profesor en la version nueva de la maquina (ver Caso 20b): ahora reporta Size=0x00000004 correctamente.

