<template>
	<v-form>

		<v-row>
			<v-col>

				<s-input
					type="select"
					:data="'config.project.entities[0].collisions.type'"
					label="Type"
					hint="Type"
					:items="optionsTypes"
				></s-input>

			</v-col>
		</v-row>

	</v-form>
</template>

<script>
	module.exports = {
		computed: {
			optionsTypes() {
				let options = []
				for (let type of this.$root.data.data.config.combined.enums.types) {
					options.push({
						text: type.name,
						value: type.name
					})
				}

				options.sort(function (a,b) {
					return a['text'].localeCompare(b['text']);
				});

				return options
			}
		}
	}
</script>

<style scoped>
</style>