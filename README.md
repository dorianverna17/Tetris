Tetris Game
Verna Dorian-Alexandru

Proiectul consta intr-un joc Tetris realizat cu ajutorul unei matrice de LED-uri.
Jucatorul trebuie sa aseze piesele care apar in matrice astfel incat aceasta sa
nu fie acoperita de ele. Cu cat jucatorul poate sa amane completarea cu piese a
spatiului pe care il are la dispozitie, cu atat acesta este mai aproape de a castiga.
Jucatorul castiga atunci cand rezista o anumita perioada de timp data de dificultatea
jocului. Scorul se afiseaza pe un display, iar dificultatea (viteza, timpul) se
regleaza cu ajutorul unui potentiometru. Rasplata pentru castig consta intr-o
surpriza data catre jucator prin intermediul unui servomotor.

In momentul in care se pune in functiune ansamblul, pe display va aparea un mesaj
care va spune ca se poate incepe jocul odata cu apasarea unui buton. Odata inceput
jocul, jucatorul trebuie sa seteze dificultatea pe care o va avea pentru joc prin
intermediul potentiometrului (timpul cat trebuie sa reziste si viteza cu care coboara
piesele), dupa care se poate da start la joc de pe acelasi buton ca inainte. Piesele
care trebuie asezate cat mai bine pe matricea de LED-uri sunt reprezentate prin
LED-uri aprinse. Jucatorul le poate roti sau misca stanga-dreapta cu ajutorul a 4
butoane. Jocul se incheie atunci cand se termina timpul setat pentru joc sau cand
jucatorul a pierdut. Daca acesta a castigat, atunci, prin intermediul servomotorului,
jucatorului ii va fi acordat premiul.

Am inceput proiectul cu pasi mici, primul a fost generarea random a pieselor. Dupa
acest pas, am inceput sa implementez modul in care coboara acestea pe matricea de
LED-uri, urmand sa se opreasca atunci cand ajung la baza matricei. Am folosit o
structura (moving_piece) pentru a retine pozitiile pe care le detine o piesa in
miscare pe matricea mare de LED-uri (practic, pentru a face piesa sa coboare pe
matrice, incrementez coloanele aferente campurilor din moving_piece). Pe matricea
mare care simuleaza matricea de LED-uri, 0 inseamna spatiu liber, 1 inseamna piesa
care s-a oprit, iar 2 inseamna piesa care inca se misca. Matricea aceasta este
afisata tot timpul in loop pe matricea fizica. Pe matricea fizica pot sa setez doar
cate un rand intreg pe fiecare din cele 4 matrice care intra in componenta sa.
Urmatorul pas in implementare consta in implementarea utilitatii butoanelor. Am 5
butoane in total, dintre care 4 le folosesc pentru miscarea pieselor. Primele 2
butoane de pe breadboard, cele pentru miscarea stanga-dreapta a piesei, apeleaza,
atunci cand sunt apasate, functiile aferente care verifica daca se poate realiza
miscarea stanga-dreapta a piesei, iar in caz afirmativ, decrementez si incrementez
randul elementelor din moving_piece.
Dupa aceea, a trebuit a implementez rotirea pieselor pentru urmatoarele butoane (rotire
stanga, respectiv rotire dreapta). Ca dificultate, aceasta a fost cea mai grea parte
din tema. Practic, aici folosesc o matrice auxiliara unde efectuez rotirea, iar daca
aceasta este posibila, atunci translatez pozitiile care vor fi ocupate dupa rotire de
piesa in matricea originala. Apoi am implementat stergerea unor linii si shiftarea in
jos a liniilor de deasupra.
Dupa aceea, am conectat display-ul LCD si am facut posibila afisarea unor mesaje precum
cel care afiseaza scorul pe ecran, urmand ca, mai apoi, sa introduc si potentiometrul
care sa faca posibila alegerea dificultatii (numar de linii care trebuie sterse si viteza piese).
La sfarsit de tot am introdus si servomotorul pe care l-am pus intr-o cutie. In momentul
in care se castiga jocul, servomotorul deschide cutia si jucatorul isi poate lua premiul.

