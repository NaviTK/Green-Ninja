import tkinter as tk

# El mapa de texto que nos compartiste (lo usamos como base)
MAPA_INICIAL = [
    "w w w w w w w w w w w w w w w",
    "w f f . . . . . . . . . f f w",
    "w f r . . . . . . . . . r f w",
    "w . . . . . . . . . . . . . w",
    "w . . . . . m . m . . . . . w",
    "w . . . . . . . . . . . . . w",
    "w f r . . . . . . . . . r f w",
    "w f f . . . . . . . . . f f w",
    "w w w w w w w w w w w w w w w"
]

# Diccionario para traducir letras a colores visuales
COLORES = {
    'w': 'gray',   # Pared
    '.': 'white',  # Suelo vacío
    'f': 'green',  # fosa
    'r': 'blue',   # roca
    'm': 'red'     # Mapache
}

class EditorSalas:
    def __init__(self, root):
        self.root = root
        self.root.title("Editor Visual de Salas")
        self.pincel_actual = '.' # Herramienta seleccionada por defecto
        
        # Convertimos las cadenas de texto en una lista bidimensional modificable
        self.grid_data = [line.split() for line in MAPA_INICIAL]
        self.botones_grid = []

        self.crear_paleta()
        self.crear_cuadricula()
        self.crear_boton_guardar()

    def crear_paleta(self):
        """Crea los botones laterales para elegir qué elemento pintar."""
        frame_paleta = tk.Frame(self.root)
        frame_paleta.pack(side=tk.LEFT, padx=10, pady=10)
        tk.Label(frame_paleta, text="Herramientas").pack(pady=5)
        
        for letra, color in COLORES.items():
            btn = tk.Button(frame_paleta, text=letra, bg=color, width=5,
                            command=lambda l=letra: self.seleccionar_pincel(l))
            btn.pack(pady=2)

    def seleccionar_pincel(self, letra):
        self.pincel_actual = letra

    def crear_cuadricula(self):
        """Crea la cuadrícula central interactiva basada en el txt."""
        frame_grid = tk.Frame(self.root)
        frame_grid.pack(side=tk.RIGHT, padx=10, pady=10)
        
        for y, fila in enumerate(self.grid_data):
            fila_botones = []
            for x, valor in enumerate(fila):
                color = COLORES.get(valor, 'white')
                # Botón sin texto, solo color. Al hacer clic, ejecuta pintar_celda
                btn = tk.Button(frame_grid, text="", bg=color, width=3, height=1,
                                command=lambda _y=y, _x=x: self.pintar_celda(_y, _x))
                btn.grid(row=y, column=x, padx=1, pady=1)
                fila_botones.append(btn)
            self.botones_grid.append(fila_botones)

    def pintar_celda(self, y, x):
        """Cambia el valor en los datos y actualiza el color del botón."""
        self.grid_data[y][x] = self.pincel_actual
        color = COLORES.get(self.pincel_actual, 'white')
        self.botones_grid[y][x].config(bg=color)

    def crear_boton_guardar(self):
        """Botón en la parte inferior para exportar a txt."""
        btn_guardar = tk.Button(self.root, text="Guardar a sala.txt", bg="lightgreen", command=self.guardar)
        btn_guardar.pack(side=tk.BOTTOM, pady=20)

    def guardar(self):
        """Convierte la lista modificada de nuevo a formato de texto y la guarda."""
        with open("sala.txt", "w") as f:
            for fila in self.grid_data:
                f.write(" ".join(fila) + "\n")
        print("¡Archivo guardado con éxito como sala.txt!")

if __name__ == "__main__":
    ventana = tk.Tk()
    app = EditorSalas(ventana)
    ventana.mainloop()