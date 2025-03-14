
---

### ParallelFirewall

```markdown
# ParallelFirewall

## Prezentare generală
ParallelFirewall este un firewall multi-threaded dezvoltat în C ca proiect de facultate.
Proiectul se axează pe filtrarea pachetelor în timp real și procesarea paralelă pentru a asigura securitatea rețelelor.

## Funcționalități
- Capturare și analiză în timp real a pachetelor de rețea
- Filtrare bazată pe reguli predefinite (allow/deny)
- Procesare paralelă cu ajutorul thread-urilor
- Logging detaliat al traficului și al evenimentelor

## Cerințe
- Compilator C (de ex. GCC)
- Sistem de operare compatibil POSIX
- Biblioteca libpcap (pentru capturarea pachetelor)
- Biblioteca pthread

**Instalare libpcap pe Debian/Ubuntu:**
```bash
sudo apt-get install libpcap-dev
