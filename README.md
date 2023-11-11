**Saptamana 6: Fișiere. Directoare**
Se va scrie un program în limbajul C ce va prelucra un fișier de intrare ce reprezinta o
imagine in format BMP și va realiza o serie de statistici pe baza acestui fișier. Programul
va primi un parametru și se va apela după cum urmează:
            ./program <fisier_intrare>
Programul va verifica faptul că a primit un singur argument, precum și tipul acestuia, iar
în caz contrar va afișa un mesaj de eroare 
            ”Usage ./program <fisier_intrare>”.
Programul trebuie sa citeasca header-ul fisierului BMP și sa extraga valoarea înălțimii,
respectiv a lungimii pentru imaginea data.
Programul va crea un fișier cu numele statistica.txt în care vor fi scrise
următoarele informații:
            nume fisier: poza.bmp
            inaltime: 1920
            lungime: 1280 
            dimensiune: <dimensiune in octeti>
            identificatorul utilizatorului: <user id>
            timpul ultimei modificari: 28.10.2023
            contorul de legaturi: <numar legaturi>
            drepturi de acces user: RWX
            drepturi de acces grup: R–-
            drepturi de acces altii: ---
Se vor folosi doar apeluri sistem pentru lucrul cu fișierele (open, read, write, close, stat,
fstat, lstat... etc). Nu se permite folosirea funcțiilor din biblioteca standard stdio pentru
lucrul cu fisiere (fopen, fread, fwrite, fclose... etc). Se permite folosirea funcției sprintf
pentru a obține un string formatat pentru a putea fi scris în fișier folosind apelul sistem
"write".

**Saptamana 7: Fișiere. Directoare**
Sa se modifice programul din saptamana anterioare astfel incat acesta va primi ca și
parametru calea către un director (în locul unei imagini) și prin urmare se va apela astfel:
              ./program <director_intrare>
Programul va trebui sa parcurga directorul dat ca parametru și va scrie în cadrul fișierului
statistica.txt în funcție de tipul intrării curente astfel:
- dacă este un fișier obișnuit cu extensia .bmp vor scrie informațiile de mai sus
- dacă este un fișier obișnuit, dar fără extensia .bmp,va scrie aceleași informații
ca și în cazul anterior, însă fără lungime și înălțime
- dacă este o legatura simbolica ce indica spre un fișier obișnuit va scrie
următoarele informații:
              nume legatura: nume
              dimensiune legatura: dimensiunea legaturii
              dimensiune fisier dimensiunea fisierului target
              drepturi de acces user legatura: RWX
              drepturi de acces grup legatura: R–-
              drepturi de acces altii legatura: ---
- dacă este un director se vor scrie următoarele informații:
              nume director: director
              identificatorul utilizatorului: <user id>
              drepturi de acces user: RWX
              drepturi de acces grup: R–-
              drepturi de acces altii: ---
- pentru orice alte cazuri nu se va scrie nimic în fișierul statistica.txt
