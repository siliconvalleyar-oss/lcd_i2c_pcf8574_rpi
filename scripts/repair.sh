#!/bin/bash

# Script para resetear el módulo LCD I2C PCF8574
# Autor: Senior Embedded Software Engineer
# Fecha: 2024

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   RESET DE LCD I2C PCF8574${NC}"
echo -e "${BLUE}========================================${NC}"

# Configuración
I2C_BUS=${1:-1}  # Bus I2C (por defecto 1)
I2C_ADDR=${2:-0x27}  # Dirección (por defecto 0x27)

echo -e "${YELLOW}Bus I2C:${NC} /dev/i2c-${I2C_BUS}"
echo -e "${YELLOW}Dirección:${NC} ${I2C_ADDR}"

# Función para verificar si i2c-tools está instalado
check_i2c_tools() {
    if ! command -v i2cset &> /dev/null; then
        echo -e "${RED}❌ i2c-tools no está instalado.${NC}"
        echo -e "${YELLOW}Instalando...${NC}"
        sudo apt-get update
        sudo apt-get install -y i2c-tools
    fi
}

# Función para verificar el bus I2C
check_i2c_bus() {
    if [ ! -e "/dev/i2c-${I2C_BUS}" ]; then
        echo -e "${RED}❌ Bus I2C ${I2C_BUS} no existe.${NC}"
        echo -e "${YELLOW}Verificando buses disponibles:${NC}"
        ls -la /dev/i2c-*
        return 1
    fi
    return 0
}

# Función para verificar la dirección I2C
check_i2c_address() {
    echo -e "${YELLOW}Verificando dirección ${I2C_ADDR}...${NC}"
    if sudo i2cdetect -y ${I2C_BUS} | grep -q "${I2C_ADDR:2}"; then
        echo -e "${GREEN}✅ LCD encontrado en ${I2C_ADDR}${NC}"
        return 0
    else
        echo -e "${RED}❌ No se encontró LCD en ${I2C_ADDR}${NC}"
        echo -e "${YELLOW}Dispositivos detectados:${NC}"
        sudo i2cdetect -y ${I2C_BUS}
        return 1
    fi
}

# Función para resetear el LCD
reset_lcd() {
    echo -e "${YELLOW}Reseteando LCD...${NC}"
    
    # Secuencia de reset para el PCF8574
    # 1. Apagar backlight
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00
    sleep 0.1
    
    # 2. Encender backlight (sin otros pines)
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x08
    sleep 0.1
    
    # 3. Secuencia de inicialización (simula el power-on)
    # Enviar 0x03 tres veces (8-bit mode)
    for i in {1..3}; do
        sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x30  # 0x03 << 4
        sleep 0.01
        sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x34  # Pulse EN
        sleep 0.01
        sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x30
        sleep 0.005
    done
    
    # 4. Cambiar a 4-bit mode
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x20  # 0x02 << 4
    sleep 0.01
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x24  # Pulse EN
    sleep 0.01
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x20
    sleep 0.005
    
    # 5. Function set (4-bit, 2 líneas, 5x8)
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x20
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x24
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x20
    sleep 0.001
    
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x80
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x84
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x80
    sleep 0.001
    
    # 6. Display on (con backlight)
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x08
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x0C
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x08
    sleep 0.001
    
    # 7. Clear display
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x04
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00
    sleep 0.003
    
    # 8. Entry mode
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x60
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x64
    sleep 0.001
    sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x60
    sleep 0.001
    
    echo -e "${GREEN}✅ LCD reseteado exitosamente${NC}"
}

# Función para mostrar el estado del LCD
show_status() {
    echo -e "${YELLOW}Estado del bus I2C:${NC}"
    sudo i2cdetect -y ${I2C_BUS}
    echo ""
    echo -e "${YELLOW}Configuración actual:${NC}"
    echo "  Bus: /dev/i2c-${I2C_BUS}"
    echo "  Dirección: ${I2C_ADDR}"
    echo "  Backlight: ON"
}

# Función principal
main() {
    # Verificar permisos
    if [ "$EUID" -ne 0 ]; then 
        echo -e "${YELLOW}⚠️  Ejecutando con sudo...${NC}"
        exec sudo "$0" "$@"
        exit
    fi
    
    # Verificar i2c-tools
    check_i2c_tools
    
    # Verificar bus I2C
    if ! check_i2c_bus; then
        echo -e "${RED}❌ Error: Bus I2C no disponible${NC}"
        exit 1
    fi
    
    # Verificar dirección I2C
    if ! check_i2c_address; then
        echo -e "${YELLOW}Probando dirección alternativa 0x3F...${NC}"
        I2C_ADDR="0x3F"
        if ! check_i2c_address; then
            echo -e "${RED}❌ No se encontró LCD en ninguna dirección${NC}"
            exit 1
        fi
    fi
    
    # Mostrar menú
    echo ""
    echo -e "${BLUE}Seleccione una opción:${NC}"
    echo "  1) Resetear LCD (completo)"
    echo "  2) Resetear backlight solamente"
    echo "  3) Mostrar estado"
    echo "  4) Salir"
    echo ""
    read -p "Opción [1-4]: " option
    
    case $option in
        1)
            reset_lcd
            show_status
            ;;
        2)
            echo -e "${YELLOW}Resetear backlight...${NC}"
            sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00
            sleep 0.5
            sudo i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x08
            echo -e "${GREEN}✅ Backlight reseteado${NC}"
            ;;
        3)
            show_status
            ;;
        4)
            echo -e "${GREEN}¡Hasta luego!${NC}"
            exit 0
            ;;
        *)
            echo -e "${RED}Opción inválida${NC}"
            ;;
    esac
}

# Ejecutar función principal
main
